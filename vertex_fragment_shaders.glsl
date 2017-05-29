#version 330

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 position;    // coordonnees du sommet
layout(location = 1) in vec2 textcoord;     // coordonnees de texture du sommet
layout(location = 2) in vec3 normal;     // normale du sommet

uniform mat4 mvpMatrix;    
uniform mat4 mvMatrix;
uniform mat4 normalMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 viewInvMatrix;
uniform mat4 lightPos;
uniform mat4 lightProj;

out vec2 vTextCoord; // texture
out vec3 vNormal; // normale
out vec3 source; // source lumineuse
out vec3 camera; // position de la caméra
out vec4 vPositionMVP; 
out vec4 vPositionM;
out vec4 vPositionLightSpace; // Position du vertex dans le repère de la lumière projeté

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
in vec3 source; // position de la source lumineuse
in vec3 camera; // position de la caméra
in vec4 vPositionM;
in vec4 vPositionMVP;
in vec4 vPositionLightSpace; // Position du vertex dans le repère de la lumière projeté 

// Textures et shadowMap
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D shadowMap;

// Position et direction des phares
uniform vec3 spotP1Pos;
uniform vec3 spotP2Pos;
uniform vec3 spotP1Dir;
uniform vec3 spotP2Dir;

// disque de poisson pour l'anti-aliasing des ombres
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

// Retourne un flottant aléatoire à partir d'un vec3 et d'un entier
float random(vec3 seed, int i){
    vec4 seed4 = vec4(seed,i);
    float dot_product = dot(seed4, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}

// Fonction de calcul des ombres
float ShadowCalculation(vec4 fragPosLightSpace){
    // homogénéisation et mise dans les coordonnées de texture
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float visibility=1.0;

    // bias (décalage) pour éviter l'"acné d'ombre"
    vec3 l = camera - vPositionM.xyz / vPositionM.w;;
    float cos_theta = max(0, dot(normalize(vNormal), normalize(l)));
    float bias = 0.005*tan(acos(cos_theta));
    bias = clamp(bias, 0,0.01);

    // échantillonnage aléatoire autour du pixel pour réduire l'aliasing
    for (int i=0;i<4;i++){
        int index = int(16.0*random(gl_FragCoord.xyy, i))%16;        
        // si l'échantillon est dans l'ombre, on diminue la luminosité du fragment
        if ( texture( shadowMap, projCoords.xy + poissonDisk[i]/1200.0 ).r  <  projCoords.z-bias ){
            visibility-= 0.1;
        }
    }

    return visibility;
}

// Fonction de calcul de l'éclairage des phares
vec3 getSpotsLight(){
    int on = 1;
    vec3 spotLight = vec3(0.0, 0.0, 0.0);

    // Phares activés?
    if(on == 1){
        // paramètres & position du fragment dans le repère monde
        float coneCos = 0.85;
        float attenuation = 0.4;
        vec3 pos = vPositionM.xyz / vPositionM.w;

        // Variables utiles
        float dist, cos, dif, factor;
        vec3 lSpot;

        // PHARE DU JOUEUR 1
        vec3 spotLightP1 = vec3(0.0, 0.0, 0.0);

        dist = distance(pos, spotP1Pos);
        lSpot = normalize(pos - spotP1Pos);

        cos = dot(spotP1Dir, lSpot);
        dif = 1.0 - coneCos;
        factor = clamp((cos - coneCos)/dif, 0.0, 1.0);

        if(cos > coneCos)
            spotLightP1 = vec3(1.0, 1.0, 1.0) * factor/(dist * attenuation);

        // PHARE DU JOUEUR 2
        vec3 spotLightP2 = vec3(0.0, 0.0, 0.0);

        dist = distance(pos, spotP2Pos);
        lSpot = normalize(pos - spotP2Pos);

        cos = dot(spotP2Dir, lSpot);
        dif = 1.0 - coneCos;
        factor = clamp((cos - coneCos)/dif, 0.0, 1.0);

        if(cos > coneCos)
            spotLightP2 = vec3(1.0, 1.0, 1.0) * factor/(dist * attenuation);

        // SOMME
        return spotLightP1 + spotLightP2;
    }

}

void main( )
{
    // lumière ambiante
    vec3 ambiantLight = vec3(0.1, 0.1, 0.1);

    // lumière diffuse (soleil avec cycle jour / nuit)
    vec3 l = source - vPositionM.xyz / vPositionM.w;;
    // calculer le cosinus de l'angle entre les 2 directions
    float cos_theta = max(0, dot(normalize(vNormal), normalize(l)));
    vec3 diffuseLight = vec3(1.0, 1.0, 1.0) * cos_theta;

    // phares
    vec3 spotLight = getSpotsLight();
    
    // ombres
    float diffuseShadow = ShadowCalculation(vPositionLightSpace);

    // Lumière du fragment
    vec3 localLight = ambiantLight + (diffuseLight * diffuseShadow) + spotLight;

    // Couleur de fragment (sans lumière)
    vec4 grassColor = texture(texture1, vTextCoord) * texture(texture0, vTextCoord*30);
    vec4 roadColor = (1-texture(texture1, vTextCoord)) * texture(texture2, vTextCoord*30);
    vec4 color = grassColor + roadColor;
    
    // couleur finale
    fragment_color = color * vec4(localLight.xyz, 1.0);
}
#endif