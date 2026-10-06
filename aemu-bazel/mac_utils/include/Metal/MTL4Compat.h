// TODO(b/567091343): Remove these Metal 4 SDK compatibility headers once the
// hermetic macOS Xcode SDK in toolchains is updated to macOS 26+ (Xcode 26+).
#pragma once
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#ifndef MTLGPUFamilyMetal4
#define MTLGPUFamilyMetal4 ((MTLGPUFamily)5002)
#endif

typedef NSUInteger MTLStages;
typedef NSUInteger MTL4VisibilityOptions;
#define MTL4VisibilityOptionResourceAlias ((MTL4VisibilityOptions)1)

typedef NSUInteger MTL4TimestampGranularity;
#define MTL4TimestampGranularityRelaxed ((MTL4TimestampGranularity)0)

typedef NSUInteger MTL4CounterHeapType;
#define MTL4CounterHeapTypeTimestamp ((MTL4CounterHeapType)0)

typedef NSUInteger MTLSamplerReductionMode;

typedef struct {
    uint64_t bufferAddress;
    uint64_t length;
} MTL4BufferRange;

typedef struct {
    uint64_t timestamp;
} MTL4TimestampHeapEntry;

__attribute__((weak_import))
@interface MTL4CounterHeapDescriptor : NSObject
@property (nonatomic, assign) MTL4CounterHeapType type;
@property (nonatomic, assign) NSUInteger count;
@end

@protocol MTL4CounterHeap <NSObject>
@end

__attribute__((weak_import))
@interface MTL4ArgumentTableDescriptor : NSObject
@property (nonatomic, assign) NSUInteger maxBufferBindCount;
@end

@protocol MTL4ArgumentTable <NSObject>
- (void)setAddress:(uint64_t)address atIndex:(NSUInteger)index;
@end

@protocol MTL4CommandAllocator <NSObject>
- (void)reset;
@end

__attribute__((weak_import))
@interface MTL4FunctionDescriptor : NSObject
@end

__attribute__((weak_import))
@interface MTL4LibraryFunctionDescriptor : MTL4FunctionDescriptor
@property (nonatomic, copy) NSString *name;
@property (nonatomic, retain) id<MTLLibrary> library;
@end

__attribute__((weak_import))
@interface MTL4LibraryDescriptor : NSObject
@property (nonatomic, copy) NSString *source;
@property (nonatomic, retain) MTLCompileOptions *options;
@end

__attribute__((weak_import))
@interface MTL4CompilerDescriptor : NSObject
@end

__attribute__((weak_import))
@interface MTL4ComputePipelineDescriptor : NSObject
@property (nonatomic, retain) MTL4FunctionDescriptor *computeFunctionDescriptor;
@property (nonatomic, assign) NSUInteger maxTotalThreadsPerThreadgroup;
@end

__attribute__((weak_import))
@interface MTL4RenderPipelineColorAttachmentDescriptor : NSObject
@property (nonatomic, assign) MTLPixelFormat pixelFormat;
@end

__attribute__((weak_import))
@interface MTL4RenderPipelineColorAttachmentDescriptorArray : NSObject
- (MTL4RenderPipelineColorAttachmentDescriptor *)objectAtIndexedSubscript:(NSUInteger)attachmentIndex;
@end

__attribute__((weak_import))
@interface MTL4RenderPipelineDescriptor : NSObject
@property (nonatomic, retain) MTL4FunctionDescriptor *vertexFunctionDescriptor;
@property (nonatomic, retain) MTL4FunctionDescriptor *fragmentFunctionDescriptor;
@property (nonatomic, assign) MTLPrimitiveTopologyClass inputPrimitiveTopology;
@property (readonly) MTL4RenderPipelineColorAttachmentDescriptorArray *colorAttachments;
@property (nonatomic, assign) NSUInteger rasterSampleCount;
@property (nonatomic, assign, getter=isRasterizationEnabled) BOOL rasterizationEnabled;
@property (nonatomic, assign) NSUInteger maxVertexAmplificationCount;
@end

@protocol MTL4Compiler <NSObject>
- (id<MTLLibrary>)newLibraryWithDescriptor:(MTL4LibraryDescriptor *)descriptor error:(NSError **)error;
- (id<MTLComputePipelineState>)newComputePipelineStateWithDescriptor:(MTL4ComputePipelineDescriptor *)descriptor compilerTaskOptions:(id)options error:(NSError **)error;
- (id<MTLRenderPipelineState>)newRenderPipelineStateWithDescriptor:(MTL4RenderPipelineDescriptor *)descriptor compilerTaskOptions:(id)options error:(NSError **)error;
@end

__attribute__((weak_import))
@interface MTL4RenderPassDescriptor : NSObject
@property (readonly) MTLRenderPassColorAttachmentDescriptorArray *colorAttachments;
@property (nonatomic, copy) MTLRenderPassDepthAttachmentDescriptor *depthAttachment;
@property (nonatomic, copy) MTLRenderPassStencilAttachmentDescriptor *stencilAttachment;
@property (nonatomic, assign) NSUInteger renderTargetArrayLength;
@property (nonatomic, assign) NSUInteger renderTargetWidth;
@property (nonatomic, assign) NSUInteger renderTargetHeight;
@property (nonatomic, assign) NSUInteger defaultRasterSampleCount;
@property (nonatomic, retain) id<MTLBuffer> visibilityResultBuffer;
- (void)setSamplePositions:(const MTLSamplePosition *)positions count:(NSUInteger)count;
@end

@protocol MTL4CommandEncoder <NSObject>
@property (nullable, copy, atomic) NSString *label;
- (void)endEncoding;
- (void)barrierAfterStages:(MTLStages)afterStages beforeQueueStages:(MTLStages)beforeQueueStages visibilityOptions:(MTL4VisibilityOptions)visibilityOptions;
- (void)barrierAfterEncoderStages:(MTLStages)afterEncoderStages beforeEncoderStages:(MTLStages)beforeEncoderStages visibilityOptions:(MTL4VisibilityOptions)visibilityOptions;
- (void)barrierAfterQueueStages:(MTLStages)afterQueueStages beforeStages:(MTLStages)beforeStages visibilityOptions:(MTL4VisibilityOptions)visibilityOptions;
- (void)updateFence:(id<MTLFence>)fence afterEncoderStages:(MTLStages)afterEncoderStages;
- (void)waitForFence:(id<MTLFence>)fence beforeEncoderStages:(MTLStages)beforeEncoderStages;
- (void)insertDebugSignpost:(NSString *)string;
- (void)pushDebugGroup:(NSString *)string;
- (void)popDebugGroup;
@end

@protocol MTL4ComputeCommandEncoder <MTL4CommandEncoder>
- (void)copyFromBuffer:(id<MTLBuffer>)sourceBuffer sourceOffset:(NSUInteger)sourceOffset toBuffer:(id<MTLBuffer>)destinationBuffer destinationOffset:(NSUInteger)destinationOffset size:(NSUInteger)size;
- (void)copyFromBuffer:(id<MTLBuffer>)sourceBuffer sourceOffset:(NSUInteger)sourceOffset sourceBytesPerRow:(NSUInteger)sourceBytesPerRow sourceBytesPerImage:(NSUInteger)sourceBytesPerImage sourceSize:(MTLSize)sourceSize toTexture:(id<MTLTexture>)destinationTexture destinationSlice:(NSUInteger)destinationSlice destinationLevel:(NSUInteger)destinationLevel destinationOrigin:(MTLOrigin)destinationOrigin options:(MTLBlitOption)options;
- (void)copyFromTexture:(id<MTLTexture>)sourceTexture sourceSlice:(NSUInteger)sourceSlice sourceLevel:(NSUInteger)sourceLevel sourceOrigin:(MTLOrigin)sourceOrigin sourceSize:(MTLSize)sourceSize toBuffer:(id<MTLBuffer>)destinationBuffer destinationOffset:(NSUInteger)destinationOffset destinationBytesPerRow:(NSUInteger)destinationBytesPerRow destinationBytesPerImage:(NSUInteger)destinationBytesPerImage options:(MTLBlitOption)options;
- (void)copyFromTexture:(id<MTLTexture>)sourceTexture sourceSlice:(NSUInteger)sourceSlice sourceLevel:(NSUInteger)sourceLevel sourceOrigin:(MTLOrigin)sourceOrigin sourceSize:(MTLSize)sourceSize toTexture:(id<MTLTexture>)destinationTexture destinationSlice:(NSUInteger)destinationSlice destinationLevel:(NSUInteger)destinationLevel destinationOrigin:(MTLOrigin)destinationOrigin;
- (void)setComputePipelineState:(id<MTLComputePipelineState>)state;
- (void)setArgumentTable:(id<MTL4ArgumentTable>)argumentTable;
- (void)dispatchThreads:(MTLSize)threadsPerGrid threadsPerThreadgroup:(MTLSize)threadsPerThreadgroup;
- (void)dispatchThreadgroupsWithIndirectBuffer:(uint64_t)indirectBufferAddress threadsPerThreadgroup:(MTLSize)threadsPerThreadgroup;
- (void)writeTimestampWithGranularity:(MTL4TimestampGranularity)granularity intoHeap:(id<MTL4CounterHeap>)heap atIndex:(NSUInteger)index;
@end

@protocol MTL4RenderCommandEncoder <MTL4CommandEncoder>
- (void)setViewports:(const MTLViewport *)viewports count:(NSUInteger)count;
- (void)setScissorRects:(const MTLScissorRect *)scissorRects count:(NSUInteger)count;
- (void)setRenderPipelineState:(id<MTLRenderPipelineState>)pipelineState;
- (void)setDepthStencilState:(nullable id<MTLDepthStencilState>)depthStencilState;
- (void)setStencilFrontReferenceValue:(uint32_t)frontReferenceValue backReferenceValue:(uint32_t)backReferenceValue;
- (void)setFrontFacingWinding:(MTLWinding)frontFacingWinding;
- (void)setCullMode:(MTLCullMode)cullMode;
- (void)setVisibilityResultMode:(MTLVisibilityResultMode)mode offset:(NSUInteger)offset;
- (void)setDepthBias:(float)depthBias slopeScale:(float)slopeScale clamp:(float)clamp;
- (void)setDepthClipMode:(MTLDepthClipMode)depthClipMode;
- (void)setDepthTestMinBound:(float)minBound maxBound:(float)maxBound;
- (void)setVertexAmplificationCount:(NSUInteger)count viewMappings:(nullable const MTLVertexAmplificationViewMapping *)viewMappings;
- (void)setArgumentTable:(id<MTL4ArgumentTable>)argumentTable atStages:(MTLRenderStages)stages;
- (void)drawPrimitives:(MTLPrimitiveType)primitiveType vertexStart:(NSUInteger)vertexStart vertexCount:(NSUInteger)vertexCount instanceCount:(NSUInteger)instanceCount baseInstance:(NSUInteger)baseInstance;
- (void)drawIndexedPrimitives:(MTLPrimitiveType)primitiveType indexCount:(NSUInteger)indexCount indexType:(MTLIndexType)indexType indexBuffer:(uint64_t)indexBufferAddress indexBufferLength:(NSUInteger)indexBufferLength instanceCount:(NSUInteger)instanceCount baseVertex:(NSInteger)baseVertex baseInstance:(NSUInteger)baseInstance;
- (void)drawPrimitives:(MTLPrimitiveType)primitiveType indirectBuffer:(uint64_t)indirectBufferAddress;
- (void)drawIndexedPrimitives:(MTLPrimitiveType)primitiveType indexType:(MTLIndexType)indexType indexBuffer:(uint64_t)indexBufferAddress indexBufferLength:(NSUInteger)indexBufferLength indirectBuffer:(uint64_t)indirectBufferAddress;
- (void)writeTimestampWithGranularity:(MTL4TimestampGranularity)granularity afterStage:(MTLRenderStages)stage intoHeap:(id<MTL4CounterHeap>)heap atIndex:(NSUInteger)index;
- (void)setColorStoreAction:(MTLStoreAction)storeAction atIndex:(NSUInteger)colorAttachmentIndex;
- (void)setDepthStoreAction:(MTLStoreAction)storeAction;
- (void)setStencilStoreAction:(MTLStoreAction)storeAction;
@end

@protocol MTL4CommandBuffer <NSObject>
@property (nullable, copy, atomic) NSString *label;
- (void)beginCommandBufferWithAllocator:(id<MTL4CommandAllocator>)allocator;
- (void)endCommandBuffer;
- (void)resolveCounterHeap:(id<MTL4CounterHeap>)heap withRange:(NSRange)range intoBuffer:(MTL4BufferRange)bufferRange waitFence:(nullable id<MTLFence>)waitFence updateFence:(nullable id<MTLFence>)updateFence;
- (id<MTL4ComputeCommandEncoder>)computeCommandEncoder;
- (id<MTL4RenderCommandEncoder>)renderCommandEncoderWithDescriptor:(MTL4RenderPassDescriptor *)descriptor;
@end

@protocol MTL4CommitFeedback <NSObject>
@property (readonly, nullable) NSError *error;
@property (readonly) CFTimeInterval GPUStartTime;
@property (readonly) CFTimeInterval GPUEndTime;
@end

typedef void (^MTL4CommitFeedbackHandler)(id<MTL4CommitFeedback> feedback);

__attribute__((weak_import))
@interface MTL4CommitOptions : NSObject
- (void)addFeedbackHandler:(MTL4CommitFeedbackHandler)block;
@end

@protocol MTL4CommandQueue <NSObject>
- (void)addResidencySet:(id<MTLResidencySet>)residencySet;
- (void)removeResidencySet:(id<MTLResidencySet>)residencySet;
- (void)signalEvent:(id<MTLEvent>)event value:(uint64_t)value;
- (void)waitForEvent:(id<MTLEvent>)event value:(uint64_t)value;
- (void)commit:(id<MTL4CommandBuffer> const *)commandBuffers count:(NSUInteger)count options:(MTL4CommitOptions *)options;
- (void)waitForDrawable:(id<MTLDrawable>)drawable;
- (void)signalDrawable:(id<MTLDrawable>)drawable;
@end

@protocol KKMTLDeviceMetal4Ext <NSObject>
- (id<MTL4ArgumentTable>)newArgumentTableWithDescriptor:(MTL4ArgumentTableDescriptor *)descriptor error:(NSError **)error;
- (id<MTL4CommandQueue>)newMTL4CommandQueue;
- (id<MTL4Compiler>)newCompilerWithDescriptor:(MTL4CompilerDescriptor *)descriptor error:(NSError **)error;
- (uint64_t)queryTimestampFrequency;
- (id<MTL4CounterHeap>)newCounterHeapWithDescriptor:(MTL4CounterHeapDescriptor *)descriptor error:(NSError **)error;
- (id<MTL4CommandAllocator>)newCommandAllocator;
- (id)newCommandBuffer;
@end

#define MTLDevice MTLDevice, KKMTLDeviceMetal4Ext
#define MTLCopyAllDevices() ((id)MTLCopyAllDevices())

@interface MTLSamplerDescriptor (KKMetal4Ext)
@property (nonatomic, assign) MTLSamplerReductionMode reductionMode;
@end
