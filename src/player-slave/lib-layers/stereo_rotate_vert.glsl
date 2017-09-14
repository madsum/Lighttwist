uniform float B; //signed half baseline
uniform float R; //radius of the screen
uniform float distortion; //0.0 for true stereo, 1.0 for omnistereo
uniform float camangle; //y camera orientation
uniform int lighting; //if 0, lights are disabled

const float PI=3.1415926535;

float norm(in float px,in float py)
{
  return(sqrt(px*px+py*py));
}

float vnorm(in vec3 p)
{
  return(sqrt(p.x*p.x+p.y*p.y+p.z*p.z));
}

float atan2_local(in float px,in float py)
{
  return(2.0*atan(py/(norm(px,py)+px)));
}

vec3 rot_X(in vec3 p,in float angle)
{
  vec3 q;
  q.x=p.x;
  q.y=cos(angle)*p.y-sin(angle)*p.z;
  q.z=sin(angle)*p.y+cos(angle)*p.z;
  return(q);
}

vec3 rot_Y(in vec3 p,in float angle)
{
  vec3 q;
  q.x=cos(angle)*p.x+sin(angle)*p.z;
  q.y=p.y;
  q.z=-sin(angle)*p.x+cos(angle)*p.z;
  return(q);
}

vec3 rot_Z(in vec3 p,in float angle)
{
  vec3 q;
  q.x=cos(angle)*p.x-sin(angle)*p.y;
  q.y=sin(angle)*p.x+cos(angle)*p.y;
  q.z=p.z;
  return(q);
}

vec3 intersect_line_and_sphere(in float radius,in vec3 p1,in vec3 p2)
{
    vec3 l,c;
    float delta,d1;

    l=normalize(p2-p1);
    c=-p1;
    delta=dot(l,c)*dot(l,c)-dot(c,c)+radius*radius;
    d1=dot(l,c)+sqrt(delta); //there is another solution with -sqrt
    return(-c+d1*l);
}

float triSolveB(in float a,in float A,in float b)
{
    return(asin(b*sin(A)/a));
}

float triSolveCMax(in float a,in float A,in float b)
{
    float s1,s2;
    s1=PI-A-triSolveB(a,A,b);
    s2=PI-A-PI-triSolveB(a,A,b);
    if (s1>s2) return(s1);
    else return(s1);
}

vec3 slit_camera(in float r,in float e,in float f,in float px,in float pz)
{
    float d,conv,C,cx,cz,a;
    d=norm(px,pz);
    conv=atan2_local(abs(e),f);
    C=triSolveCMax(d,conv,abs(e));
    cx=px/d*abs(e);
    cz=pz/d*abs(e);
    a=sign(e)*C;
    return(rot_Y(vec3(cx,0,cz),a));
}

void main(void)
{
  //float B = 2.5; //average distance between eyes is 2.5 inches
  //float R = 75.0; //usually, set R so that B/R = 1/30
  //float R = 90.0; //the radius of our screen is 90 inches
  float Bs=B;
  float Rs=R;
  Bs/=2.0; //consider half baseline
  
  vec4 vertex = gl_ModelViewMatrix * gl_Vertex;

  //the following computes new vertex position such that projection w.r.t. origin gives correct pixel on screen

  //true stereo position
  vec3 eye=rot_Y(vec3(Bs,0,0),camangle);
  vec3 vertex_noelevat=vec3(vertex.x,0,vertex.z);
  vec3 screen_noelevat=intersect_line_and_sphere(R,eye,vertex_noelevat);
  //compute true stereo screen projection
  vec3 vertex_line=vertex.xyz-eye;
  vec3 vertex_noelevat_line=vertex_noelevat-eye;
  vec3 screen_noelevat_line=screen_noelevat-eye;
  vec3 screen_pos=eye+vertex_line/vnorm(vertex_noelevat_line)*vnorm(screen_noelevat_line);
  //compute new vertex position so that central projection gives screen_pos, the depth is kept the same
  vec3 v_stereo=screen_pos/vnorm(screen_pos)*vnorm(vertex.xyz);

  //rotational vertex position
  //compute intersection of circle having radius R and centered at the origin with the line (B,0)->(0,depth)
  float depth = norm(vertex.x,vertex.z);
  screen_pos=intersect_line_and_sphere(R,vec3(-Bs,0,0),vec3(0,0,depth));
  //rotate vertex by angle w.r.t. y-axis
  vec3 v_rotational=rot_Y(vertex.xyz,atan2_local(screen_pos.z,screen_pos.x));

  //slit vertex position
  vec3 slit_cam=slit_camera(R,-Bs,R,vertex.x,vertex.z);
  screen_pos=intersect_line_and_sphere(R,slit_cam,vec3(vertex.x,0,vertex.z));
  vec3 v_slit=vec3(screen_pos.x,vertex.y/norm(vertex.x-slit_cam.x,vertex.z-slit_cam.z)*norm(screen_pos.x-slit_cam.x,screen_pos.z-slit_cam.z),screen_pos.z)*vnorm(vertex.xyz)/R;

  //difference between omnistereo position and true stereo
  vec3 diff=v_rotational-v_stereo;
  //vec3 diff=v_rotational-v_slit;

  //apply distortion
  vec4 vertex_sheared=vertex;
  vertex_sheared.xyz=v_stereo+distortion*diff;
  //vertex_sheared.xyz=v_slit+distortion*diff;

  //update the vertex position
  gl_Position = gl_ProjectionMatrix * vertex_sheared;
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_TexCoord[1] = gl_MultiTexCoord1;

  //lighting
  vec4 shading,emission,ambient,diffuse,specular;
  emission=gl_FrontMaterial.emission;
  ambient=vec4(0.0,0.0,0.0,0);
  diffuse=vec4(0,0,0,0);
  specular=vec4(0,0,0,0);

  for (int i=0;i<8;i++)
  {
    vec3 normal     = normalize (gl_NormalMatrix * gl_Normal);
    vec3 lightpos   = gl_LightSource[i].position.xyz;
    vec3 lightdir   = normalize (vertex.xyz-lightpos);
    vec3 eyedir   = normalize (eye.xyz-vertex.xyz);

    ambient+=gl_FrontMaterial.ambient * gl_LightSource[i].ambient;

    float d = length(vertex.xyz-lightpos);
    float attenuation = 1.0 / (gl_LightSource[i].constantAttenuation + 
                               gl_LightSource[i].linearAttenuation*d +
                               gl_LightSource[i].quadraticAttenuation*d*d);

    float NdotL = max(-dot(normal, lightdir), 0.0);
    diffuse+=gl_FrontMaterial.diffuse * gl_LightSource[i].diffuse * NdotL * attenuation;

    //compute the specular term if NdotL is  larger than zero
    if (NdotL > 0.0) 
    {	
        vec3 reflection = -2.0 * normal * dot( normal,lightdir) +  lightdir;
        //vec3 reflection = reflect(lightdir, normal);
        float RdotE=max(dot(eyedir,reflection),0.0);       
        specular+=gl_FrontMaterial.specular * gl_LightSource[i].specular * pow(RdotE,gl_FrontMaterial.shininess) * attenuation;
    }
  }
  shading = emission + ambient + diffuse + specular;

  if (lighting==0) gl_FrontColor=gl_Color;
  else gl_FrontColor=shading;

  //if (depth<R) gl_FrontColor.a=0;
}

