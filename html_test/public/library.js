
var Library = {
    blitToCanvas : function (p_ctx, p_width, p_height, p_buffer) {
        let pixels = new Uint8ClampedArray(p_buffer.buffer, p_buffer.byteOffset, p_buffer.byteLength);
        let l_image_data = new ImageData(pixels, p_width, p_height)
        p_ctx.putImageData(l_image_data, 0, 0);
    }
}