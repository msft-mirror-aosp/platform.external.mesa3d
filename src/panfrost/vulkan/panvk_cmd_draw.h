/*
 * Copyright © 2024 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */

#ifndef PANVK_CMD_DRAW_H
#define PANVK_CMD_DRAW_H

#ifndef PAN_ARCH
#error "PAN_ARCH must be defined"
#endif

#include "panvk_blend.h"
#include "panvk_entrypoints.h"
#include "panvk_image.h"
#include "panvk_image_view.h"
#include "panvk_physical_device.h"

#include "vk_command_buffer.h"
#include "vk_format.h"

#include "pan_props.h"

#define MAX_VBS 16
#define MAX_RTS 8

struct panvk_cmd_buffer;

struct panvk_attrib_buf {
   mali_ptr address;
   unsigned size;
};

struct panvk_resolve_attachment {
   VkResolveModeFlagBits mode;
   struct panvk_image_view *dst_iview;
};

struct panvk_rendering_state {
   VkRenderingFlags flags;
   uint32_t layer_count;

   enum vk_rp_attachment_flags bound_attachments;
   struct {
      struct panvk_image_view *iviews[MAX_RTS];
      VkFormat fmts[MAX_RTS];
      uint8_t samples[MAX_RTS];
      struct panvk_resolve_attachment resolve[MAX_RTS];
   } color_attachments;

   struct pan_image_view zs_pview;

   struct {
      struct panvk_image_view *iview;
      VkFormat fmt;
      struct panvk_resolve_attachment resolve;
   } z_attachment, s_attachment;

   struct {
      struct pan_fb_info info;
      bool crc_valid[MAX_RTS];

#if PAN_ARCH <= 7
      uint32_t bo_count;
      struct pan_kmod_bo *bos[MAX_RTS + 2];
#endif
   } fb;

#if PAN_ARCH >= 10
   struct panfrost_ptr fbds;
   mali_ptr tiler;
#endif
};

enum panvk_cmd_graphics_dirty_state {
   PANVK_CMD_GRAPHICS_DIRTY_VS,
   PANVK_CMD_GRAPHICS_DIRTY_FS,
   PANVK_CMD_GRAPHICS_DIRTY_VB,
   PANVK_CMD_GRAPHICS_DIRTY_IB,
   PANVK_CMD_GRAPHICS_DIRTY_DESC_STATE,
   PANVK_CMD_GRAPHICS_DIRTY_RENDER_STATE,
   PANVK_CMD_GRAPHICS_DIRTY_PUSH_UNIFORMS,
   PANVK_CMD_GRAPHICS_DIRTY_STATE_COUNT,
};

struct panvk_cmd_graphics_state {
   struct panvk_descriptor_state desc_state;

   struct {
      struct vk_vertex_input_state vi;
      struct vk_sample_locations_state sl;
   } dynamic;

   struct panvk_graphics_sysvals sysvals;

#if PAN_ARCH <= 7
   struct panvk_shader_link link;
#endif

   struct {
      const struct panvk_shader *shader;
      struct panvk_shader_desc_state desc;
      bool required;
#if PAN_ARCH <= 7
      mali_ptr rsd;
#endif
   } fs;

   struct {
      const struct panvk_shader *shader;
      struct panvk_shader_desc_state desc;
#if PAN_ARCH <= 7
      mali_ptr attribs;
      mali_ptr attrib_bufs;
#endif
   } vs;

   struct {
      struct panvk_attrib_buf bufs[MAX_VBS];
      unsigned count;
   } vb;

   /* Index buffer */
   struct {
      struct panvk_buffer *buffer;
      uint64_t offset;
      uint8_t index_size;
   } ib;

   struct {
      struct panvk_blend_info info;
   } cb;

   struct panvk_rendering_state render;

   mali_ptr push_uniforms;

#if PAN_ARCH <= 7
   mali_ptr vpd;
#endif

#if PAN_ARCH >= 10
   mali_ptr tsd;
#endif

   BITSET_DECLARE(dirty, PANVK_CMD_GRAPHICS_DIRTY_STATE_COUNT);
};

#define dyn_gfx_state_dirty(__cmdbuf, __name)                                  \
   BITSET_TEST((__cmdbuf)->vk.dynamic_graphics_state.dirty,                    \
               MESA_VK_DYNAMIC_##__name)

#define gfx_state_dirty(__cmdbuf, __name)                                      \
   BITSET_TEST((__cmdbuf)->state.gfx.dirty, PANVK_CMD_GRAPHICS_DIRTY_##__name)

#define gfx_state_set_dirty(__cmdbuf, __name)                                  \
   BITSET_SET((__cmdbuf)->state.gfx.dirty, PANVK_CMD_GRAPHICS_DIRTY_##__name)

#define gfx_state_clear_all_dirty(__cmdbuf)                                    \
   BITSET_ZERO((__cmdbuf)->state.gfx.dirty)

#define gfx_state_set_all_dirty(__cmdbuf)                                      \
   BITSET_ONES((__cmdbuf)->state.gfx.dirty)

static inline uint32_t
panvk_select_tiler_hierarchy_mask(const struct panvk_physical_device *phys_dev,
                                  const struct panvk_cmd_graphics_state *state)
{
   struct panfrost_tiler_features tiler_features =
      panfrost_query_tiler_features(&phys_dev->kmod.props);
   uint32_t max_fb_wh = MAX2(state->render.fb.info.width,
                             state->render.fb.info.height);
   uint32_t last_hierarchy_bit = util_last_bit(DIV_ROUND_UP(max_fb_wh, 16));
   uint32_t hierarchy_mask = BITFIELD_MASK(tiler_features.max_levels);

   /* Always enable the level covering the whole FB, and disable the finest
    * levels if we don't have enough to cover everything.
    * This is suboptimal for small primitives, since it might force
    * primitives to be walked multiple times even if they don't cover the
    * the tile being processed. On the other hand, it's hard to guess
    * the draw pattern, so it's probably good enough for now.
    */
   if (last_hierarchy_bit > tiler_features.max_levels)
      hierarchy_mask <<= last_hierarchy_bit - tiler_features.max_levels;

   return hierarchy_mask;
}

static inline bool
fs_required(const struct panvk_cmd_graphics_state *state,
            const struct vk_dynamic_graphics_state *dyn_state)
{
   const struct pan_shader_info *fs_info =
      state->fs.shader ? &state->fs.shader->info : NULL;
   const struct vk_color_blend_state *cb = &dyn_state->cb;
   const struct vk_rasterization_state *rs = &dyn_state->rs;

   if (rs->rasterizer_discard_enable || !fs_info)
      return false;

   /* If we generally have side effects */
   if (fs_info->fs.sidefx)
      return true;

   /* If colour is written we need to execute */
   for (unsigned i = 0; i < cb->attachment_count; ++i) {
      if ((cb->color_write_enables & BITFIELD_BIT(i)) &&
          cb->attachments[i].write_mask)
         return true;
   }

   /* If alpha-to-coverage is enabled, we need to run the fragment shader even
    * if we don't have a color attachment, so depth/stencil updates can be
    * discarded if alpha, and thus coverage, is 0. */
   if (dyn_state->ms.alpha_to_coverage_enable)
      return true;

   /* If depth is written and not implied we need to execute.
    * TODO: Predicate on Z/S writes being enabled */
   return (fs_info->fs.writes_depth || fs_info->fs.writes_stencil);
}

static inline bool
cached_fs_required(ASSERTED const struct panvk_cmd_graphics_state *state,
                   ASSERTED const struct vk_dynamic_graphics_state *dyn_state,
                   bool cached_value)
{
   /* Make sure the cached value was properly initialized. */
   assert(fs_required(state, dyn_state) == cached_value);
   return cached_value;
}

#define get_fs(__cmdbuf)                                                       \
   (cached_fs_required(&(__cmdbuf)->state.gfx,                                 \
                       &(__cmdbuf)->vk.dynamic_graphics_state,                 \
                       (__cmdbuf)->state.gfx.fs.required)                      \
       ? (__cmdbuf)->state.gfx.fs.shader                                       \
       : NULL)

/* Anything that might change the value returned by get_fs() makes users of the
 * fragment shader dirty, because not using the fragment shader (when
 * fs_required() returns false) impacts various other things, like VS -> FS
 * linking in the JM backend, or the update of the fragment shader pointer in
 * the CSF backend. Call gfx_state_dirty(cmdbuf, FS) if you only care about
 * fragment shader updates. */

#define fs_user_dirty(__cmdbuf)                                                \
   (gfx_state_dirty(cmdbuf, FS) ||                                             \
    dyn_gfx_state_dirty(cmdbuf, RS_RASTERIZER_DISCARD_ENABLE) ||               \
    dyn_gfx_state_dirty(cmdbuf, CB_ATTACHMENT_COUNT) ||                        \
    dyn_gfx_state_dirty(cmdbuf, CB_COLOR_WRITE_ENABLES) ||                     \
    dyn_gfx_state_dirty(cmdbuf, CB_WRITE_MASKS) ||                             \
    dyn_gfx_state_dirty(cmdbuf, MS_ALPHA_TO_COVERAGE_ENABLE))

/* After a draw, all dirty flags are cleared except the FS dirty flag which
 * needs to be set again if the draw didn't use the fragment shader. */

#define clear_dirty_after_draw(__cmdbuf)                                       \
   do {                                                                        \
      bool __set_fs_dirty =                                                    \
         (__cmdbuf)->state.gfx.fs.shader != get_fs(__cmdbuf);                  \
      vk_dynamic_graphics_state_clear_dirty(                                   \
         &(__cmdbuf)->vk.dynamic_graphics_state);                              \
      gfx_state_clear_all_dirty(__cmdbuf);                                     \
      desc_state_clear_all_dirty(&(__cmdbuf)->state.gfx.desc_state);           \
      if (__set_fs_dirty)                                                      \
         gfx_state_set_dirty(__cmdbuf, FS);                                    \
   } while (0)

void
panvk_per_arch(cmd_init_render_state)(struct panvk_cmd_buffer *cmdbuf,
                                      const VkRenderingInfo *pRenderingInfo);

void
panvk_per_arch(cmd_force_fb_preload)(struct panvk_cmd_buffer *cmdbuf,
                                     const VkRenderingInfo *render_info);

void
panvk_per_arch(cmd_preload_render_area_border)(struct panvk_cmd_buffer *cmdbuf,
                                               const VkRenderingInfo *render_info);

void panvk_per_arch(cmd_resolve_attachments)(struct panvk_cmd_buffer *cmdbuf);

#endif