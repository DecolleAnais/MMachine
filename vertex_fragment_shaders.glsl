#version 330

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 position;    // coordonnees du sommet
layout(location = 1) in vec2 textcoord;     // coordonnees de texture du sommet
layout(location = 2) in vec3 normal;     // normale du sommet

uniform mat4 mvpMatrix;    // matrice passage du repere local au repere projectif homogene de la camera
uniform mat4 mvMatrix;
uniform mat4 normalMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 viewInvMatrix;
uniform mat4 lightPos;
uniform mat4 lightProj;

out vec4 vCameraPos;
out vec2 vTextCoord; // texture
out vec3 vNormal; // normale
out vec3 source;
out vec3 camera;
out vec4 vPositionMVP;
out vec4 vPositionM;
out vec4 vPositionLightSpace;

void main( )
{
    // obligation du vertex shader : calculer les coordonnées du sommet dans le repère projectif homogene de la camera
    vPositionM = modelMatrix * vec4(position, 1);
    vPositionMVP = mvpMatrix * vec4(position, 1);
    gl_Position = vPositionMVP; 

    // Transfert des coordonnées de texture et des normales au fragment shader
    vTextCoord = textcoord;
    vNormal = mat3(normalMatrix) * normal;

    // coordonnées du fragment dans le repère de la lumière
    vPositionLightSpace = lightProj * lightPos * vPositionM;

    // position de la camera dans le repere du monde
    vec4 camerah = viewInvMatrix * vec4(0, 0, 0, 1);
    vec4 sourceh = inverse(lightPos) * vec4(0, 0, 0, 1);

    // rappel : mat4 * vec4 = vec4 homogene, pour retrouver le point / la direction reelle, il faut diviser par la 4ieme composante
    camera = camerah.xyz / camerah.w;
    source = sourceh.xyz / sourceh.w;
}
#endif



#ifdef FRAGMENT_SHADER

out vec4 fragment_color;

in vec2 vTextCoord; 
in vec3 vNormal;
in vec4 vCameraPos;
in vec3 source;
in vec3 camera;
in vec4 vPositionM;
in vec4 vPositionMVP;
in vec4 vPositionLightSpace;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D shadowMap;

uniform vec3 spotP1Pos;
uniform vec3 spotP2Pos;
uniform vec3 spotP1Dir;
uniform vec3 spotP2Dir;

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

// float ShadowCalculation(vec4 fragPosLightSpace, vec3 light2pos){
//     // perform perspective divide
//     vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//     // Transform to [0,1] range
//     projCoords = projCoords * 0.5 + 0.5;
//     // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
//     float closestDepth = texture(shadowMap, projCoords.xy).r; 
//     // Get depth of current fragment from light's perspective
//     float currentDepth = projCoords.z;
//     // Calculate bias (based on depth map resolution and slope)
//     float bias = max(0.05 * (1.0 - dot(normalize(vNormal), normalize(light2pos))), 0.005);
//     // Check whether current frag pos is in shadow
//     // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
//     // PCF
//     float shadow = 0.0;
//     vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
//     for(int x = -1; x <= 1; ++x)
//     {
//         for(int y = -1; y <= 1; ++y)
//         {
//             float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
//             shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
//         }    
//     }
//     shadow /= 9.0;
    
//     // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
//     if(projCoords.z > 1.0)
//         shadow = 0.0;
        
//     return shadow;
// }

// Returns a random number based on a vec3 and an int.
float random(vec3 seed, int i){
    vec4 seed4 = vec4(seed,i);
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}
float ShadowCalculation(vec4 fragPosLightSpace){
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float visibility=1.0;

    // Fixed bias, or...
    // float bias = 0.005;

    // ...variable bias
    vec3 l = camera - vPositionM.xyz / vPositionM.w;;
    float cos_theta = max(0, dot(normalize(vNormal), normalize(l)));
    float bias = 0.005*tan(acos(cos_theta));
    bias = clamp(bias, 0,0.01);

    // Sample the shadow map 8 times
    for (int i=0;i<16;i++){
        // use either :
        //  - Always the same samples.
        //    Gives a fixed pattern in the shadow, but no noise
        // int index = i;
        //  - A random sample, based on the pixel's screen location. 
        //    No banding, but the shadow moves with the camera, which looks weird.
        int index = int(16.0*random(gl_FragCoord.xyy, i))%16;
        //  - A random sample, based on the pixel's position in world space.
        //    The position is rounded to the millimeter to avoid too much aliasing
        // int index = int(16.0*random(floor(vPositionM.xyz*1000.0), i))%16;
        
        // being fully in the shadow will eat up 4*0.2 = 0.8
        // 0.2 potentially remain, which is quite dark.
        if ( texture( shadowMap, projCoords.xy + poissonDisk[i]/1200.0 ).r  <  projCoords.z-bias ){
            visibility-= 0.06;
        }
    }

    // if ( texture( shadowMap, projCoords.xy ).r  <  projCoords.z-bias){
    //     visibility = 0.0;
    // }

    return visibility;
}

vec3 getSpotsLight(){
    int on = 1;
    vec3 spotLight = vec3(0.0, 0.0, 0.0);

    if(on == 1){
        // Parameters & vertex position in world reference
        float coneCos = 0.85;
        float attenuation = 0.4;
        vec3 pos = vPositionM.xyz / vPositionM.w;

        // Useful vars
        float dist, cos, dif, factor;
        vec3 lSpot;

        // PLAYER 1 SPOTLIGTH'S
        vec3 spotLightP1 = vec3(0.0, 0.0, 0.0);

        dist = distance(pos, spotP1Pos);
        lSpot = normalize(pos - spotP1Pos);

        cos = dot(spotP1Dir, lSpot);
        dif = 1.0 - coneCos;
        factor = clamp((cos - coneCos)/dif, 0.0, 1.0);

        if(cos > coneCos)
            spotLightP1 = vec3(1.0, 1.0, 1.0) * factor/(dist * attenuation);

        // PLAYER 2 SPOTLIGTH'S
        vec3 spotLightP2 = vec3(0.0, 0.0, 0.0);

        dist = distance(pos, spotP2Pos);
        lSpot = normalize(pos - spotP2Pos);

        cos = dot(spotP2Dir, lSpot);
        dif = 1.0 - coneCos;
        factor = clamp((cos - coneCos)/dif, 0.0, 1.0);

        if(cos > coneCos)
            spotLightP2 = vec3(1.0, 1.0, 1.0) * factor/(dist * attenuation);

        // SUM
        return spotLightP1 + spotLightP2;
    }

}

void main( )
{
    

    // Ambiant light
    vec3 ambiantLight = vec3(0.1, 0.1, 0.1);

    // Diffuse light
    vec3 l = source - vPositionM.xyz / vPositionM.w;;
    // calculer le cosinus de l'angle entre les 2 directions
    float cos_theta = max(0, dot(normalize(vNormal), normalize(l)));
    vec3 diffuseLight = vec3(1.0, 1.0, 1.0) * cos_theta;

    // Spotlights
    vec3 spotLight = getSpotsLight();
    
    // Shadows
    float diffuseShadow = ShadowCalculation(vPositionLightSpace);

    // Light on pixel
    // vec3 localLight = (ambiantLight * 10 * diffuseShadow) + spotLight;
    // vec3 localLight = ambiantLight + diffuseLight + spotLight;
    vec3 localLight = ambiantLight + (diffuseLight * diffuseShadow) + spotLight;

    // Colors
    vec4 grassColor = texture(texture1, vTextCoord) * texture(texture0, vTextCoord*30);
    vec4 roadColor = (1-texture(texture1, vTextCoord)) * texture(texture2, vTextCoord*30);
    vec4 color = grassColor + roadColor;
    
    // Color and light
    fragment_color = color * vec4(localLight.xyz, 1.0);
}
#endif