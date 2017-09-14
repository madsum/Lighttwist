uniform sampler2D lut_tex;
uniform sampler2D blend_tex;
uniform sampler2D texture;
uniform float fade;
uniform bool enableLUT;
//uniform vec3 gamma;
uniform float gamma0;
uniform float gamma1;
uniform float gamma2;
//uniform mat3 color;
uniform float color0;
uniform float color1;
uniform float color2;
uniform float color3;
uniform float color4;
uniform float color5;
uniform float color6;
uniform float color7;
uniform float color8;
uniform float xRttTexMin;
uniform float xRttTexMax;
uniform float yRttTexMin;
uniform float yRttTexMax;
uniform float xLutScale;
uniform float yLutScale;
uniform float lutxmin;
uniform float lutxmax;

uniform float slit;
uniform mat3 homography;

uniform sampler2D alphaTex;
uniform int useAlpha;

void main(void)
{

    //******** LUT *******************************

    //Recuperation de la couleur
    // lut_tex est GL_LUMINANCE_ALPHA -> r=g=b=luminance=x, a=y

    vec2 lutCoord = texture2D( lut_tex, gl_TexCoord[0].xy ).ra;

    float olutx=lutCoord.x*(lutxmax-lutxmin)+lutxmin;
    if (olutx>1.0) olutx-=1.0;
    if (olutx<(180.0-slit)/360.0 || olutx>(180.0+slit)/360.0) 
    {
      gl_FragColor.rgba = vec4(0,0,0,1);
    }
    else 
    {
      if (enableLUT)
      {
          lutCoord.x = lutCoord.x * xLutScale * (xRttTexMax - xRttTexMin) + xRttTexMin * xLutScale;
          lutCoord.y = lutCoord.y * yLutScale * (yRttTexMax - yRttTexMin) + yRttTexMin * yLutScale;

          vec3 luttmp = vec3(lutCoord.x,lutCoord.y,1.0);

          luttmp = luttmp * homography;
          lutCoord.x=luttmp.x/luttmp.z;
          lutCoord.y=luttmp.y/luttmp.z;
      }
      else
      {
          lutCoord = gl_TexCoord[0].xy;
      }

      vec4 outTex = texture2D( texture, lutCoord.xy ).rgba;
      vec3 outColor = outTex.rgb;

      //********* BLEND **********************************

      float alpha = texture2D( blend_tex, gl_TexCoord[0].xy ).r;

      vec3 alphagamma;
      alphagamma.x = pow( alpha, 1.0 / gamma0 );
      alphagamma.y = pow( alpha, 1.0 / gamma1 );
      alphagamma.z = pow( alpha, 1.0 / gamma2 );

      outColor *= vec3( alphagamma.x, alphagamma.y, alphagamma.z );
      //outColor *= vec3( alpha, alpha, alpha );

      gl_FragColor.rgb = outColor;
      if (alpha == 0)
      {
         gl_FragColor.a = 1.0;
      } 
      else
      {
         if (useAlpha != 0)
         {
           fade *= texture2D( alphaTex, lutCoord.xy ).r;
         }
         gl_FragColor.a = outTex.a*fade;
      }
    }   
}

