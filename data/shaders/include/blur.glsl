//TODO: blur kernel defined by a texture

vec4 blur(sampler2D _source, vec2 _uvs, float _blur_size, vec2 _texel_size)
{
  vec4 blurred;

  //up down left right
  blurred += texture2D(_source, _uvs + _blur_size * vec2(0.0,  _texel_size.y));
  blurred += texture2D(_source, _uvs + _blur_size * vec2(0.0, -_texel_size.y));
  blurred += texture2D(_source, _uvs + _blur_size * vec2(-_texel_size.x, 0.0));
  blurred += texture2D(_source, _uvs + _blur_size * vec2( _texel_size.x, 0.0));

  //diagonals
  blurred += texture2D(_source, _uvs + _blur_size * 0.707 * vec2( _texel_size.x,  _texel_size.y));
  blurred += texture2D(_source, _uvs + _blur_size * 0.707 * vec2( _texel_size.x, -_texel_size.y));
  blurred += texture2D(_source, _uvs + _blur_size * 0.707 * vec2(-_texel_size.x, -_texel_size.y));
  blurred += texture2D(_source, _uvs + _blur_size * 0.707 * vec2(-_texel_size.x,  _texel_size.y));

  blurred /= 8.0;

  return blurred;
}
