In Metal, the **origin** of the pixel coordinate system of a framebuffer attachment is defined at the
**top left** corner.



MBP Early 2015 with Intel Iris Graphics 6100

do not support MTLPixelFormatDepth24Unorm_Stencil8 (isDepth24Stencil8PixelFormatSupported return NO)

分开设置depth buffer(MTLPixelFormatDepth32Float)和stencil buffer(MTLPixelFormatStencil8)的贴图会报错

> failed assertion `When depth and stencil are used together, the texture bound to the depth and stencil render pass attachments must have a combined depth+stencil pixel format.'

只能使用MTLPixelFormatDepth32Float_Stencil8