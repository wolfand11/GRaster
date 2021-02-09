#ifndef GGRAPHICLIBDEFINE_H
#define GGRAPHICLIBDEFINE_H

enum GFrontFace
{
    kCounterClockwise,
    kClockwise
};

enum GCullFaceType
{
    kFTNone,
    kFTFront,
    kFTBack,
    kFTShaderSetting
};

enum GPrimitiveType
{
    kPoints,
    kLines,
    kTriangles,
};

enum GDatumType
{
    kFloat,
    kDouble,
    kInt,
    kUInt,
    kChar,
    kUChar
};

enum GPolygonMode
{
    kPMPoint,
    kPMLine,
    kPMFill,
};

enum GRenderBufferType
{
    kRBNon=-1,
    kRBFront,
    kRBBack,
    //kFrontAndBack,
    kRBColorAttachment0,
    kRBColorAttachment1,
    kRBColorAttachment2,
    kRBColorAttachment3,
    kRBColorAttachment4,
    kRBMax,
};

enum GDataBufferType
{
    kArrayBuffer,
    kElementArrayBuffer,
    kPixelPackBuffer,
    kPixelUnpackBuffer,
    kTextureBuffer,
    kTransformFeedbackBuffer,
    kUniformBuffer,
};

enum GMipmapType
{
    kMipmapOff,
    kMipmapIsotropy,
    kMipmapAnisotropy
};

enum GFrustumPlaneType
{
    kFPTFront,
    kFPTBack,
    kFPTLeft,
    kFPTRight,
    kFPTBottom,
    kFPTTop,
    kFPTW,
};

#endif // GGRAPHICLIBDEFINE_H
