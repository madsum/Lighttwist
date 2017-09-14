uniform sampler2D lut_tex;
uniform sampler2D blend_tex;
uniform sampler2D texture;
uniform float viewAngle;
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
uniform float xLutScale;
uniform float yLutScale;
uniform float lutxmin;
uniform float lutxmax;

uniform float nbCams;
uniform float slit;

uniform mat3 homography;

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
      vec2 cylCoord;
      if (enableLUT) 
      {
    	lutCoord.x = lutCoord.x * xLutScale;
        lutCoord.y = lutCoord.y * yLutScale;

        vec3 luttmp = vec3(lutCoord.x,lutCoord.y,1.0);

        luttmp = luttmp * homography;
        lutCoord.x=luttmp.x/luttmp.z;
        lutCoord.y=luttmp.y/luttmp.z;

	    //******* CYLINDER ******************************
  
        //float va = viewAngle;
        float va = viewAngle / float(nbCams);
        float curCam = floor( lutCoord.x * nbCams );
        float lutCoordXNorm = mod(nbCams*lutCoord.x, 1.0);

        float pi = 3.141592653589793;
        //float theta = (pi/2.0) - ( lutCoord.x - 0.5 ) * va;
        float theta = (pi/2.0) - ( lutCoordXNorm - 0.5 ) * va;

	    //d = plane distance  
        float d = 1.0 / tan(va / 2.0);

        // yScale = ( .5 * cos(va/2.0) / d ) * 2 -> scale to avoid clipping problems  
        float yScale = cos(va/2.0) / d ;

        // modify y to avoid clipping problems	
	    float y = lutCoord.y * yScale + 0.5 * ( 1.0 - yScale );

	    vec3 p0 = vec3( 0, 0.5, 0 );                   // camera center
	    vec3 p1 = vec3( cos(theta), y, sin(theta) );   // point on cyl. (p1-p0) to be intersected w/plane
	    vec3 v0 = vec3( 0, 0, d );  // point on the plane, plane placed so that intersection are between 0 and 1.
	    vec3 planeNorm = vec3( 0, 0, -1 );           // normal to the plane

	    float id = dot( planeNorm, v0 - p0 ) / dot ( planeNorm, p1 - p0 ); // intersect distance
	    vec3 ip = p0 + id * ( p1 - p0 ); // intersection point
 
        //cylCoord = vec2( ((ip.x + 1.0)/2.0), 1.0-ip.y ); // loader la texture a l'envers!
        cylCoord = vec2( ((ip.x + 1.0)/2.0), ip.y ); // loader la texture a l'endroit!!
	
        cylCoord.x = (curCam + cylCoord.x) / nbCams; // put x to it's rightful place
      }
      else 
      {
        cylCoord = gl_TexCoord[0].xy;
      }
      //vec4 outColor = vec4(cylCoord.x, 0, 0, 1);
      vec4 outColor = texture2D( texture, cylCoord ).rgba;

      //********* BLEND **********************************

      //vec3 tmpColor=outColor;
      //outColor.x = tmpColor.x*color0+tmpColor.y*color1+tmpColor.z*color2;
      //outColor.y = tmpColor.x*color3+tmpColor.y*color4+tmpColor.z*color5;
      //outColor.z = tmpColor.x*color6+tmpColor.y*color7+tmpColor.z*color8;

      float alpha = texture2D( blend_tex, gl_TexCoord[0].xy ).r;

      vec3 alphagamma;
      alphagamma.x = pow( alpha, 1.0 / gamma0 );
      alphagamma.y = pow( alpha, 1.0 / gamma1 );
      alphagamma.z = pow( alpha, 1.0 / gamma2 );

      outColor *= vec4( alphagamma.x, alphagamma.y, alphagamma.z, 1.0);
      //outColor *= vec4( alpha, alpha, alpha, 1.0);

      //mix le alpha de la scene avec le fade
      outColor.a *= fade;
	
      gl_FragColor = outColor;
    }
}

