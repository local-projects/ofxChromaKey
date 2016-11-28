#include "ofxChromaKey.h"

ofxChromaKey::ofxChromaKey(){
    keyColor = ofColor(0, 255, 0);
    threshold = 0.1;
	float gamma = 1.0;
    string vertex;
    string fragment;
    
    if(ofIsGLProgrammableRenderer()){
        vertex = R"GLSL(
        #version 150
        uniform mat4 modelViewProjectionMatrix;
        in vec4 position;
        in vec2 texcoord;
        out vec2 texCoordVarying;
        void main()
        {
            texCoordVarying = texcoord;
            gl_Position = modelViewProjectionMatrix * position;
        }
        )GLSL";
        
        fragment = R"GLSL(
        #version 150
        uniform sampler2DRect tex0;
        uniform float threshold;
		uniform float gamma;
        uniform vec3 chromaKeyColor;
        in vec2 texCoordVarying;
        out vec4 outputColor;
        void main()
        {
            vec4 texel0 = texture(tex0, texCoordVarying);
            float diff = length(chromaKeyColor - texel0.rgb);
            if(diff < threshold){
                outputColor = vec4(texel0.rgb, pow(diff / threshold, gamma));
            }else{
                outputColor = texel0;
            }
        }
        )GLSL";
    }else{
        vertex = R"GLSL(
        #version 120
        varying vec2 texCoordVarying;
        void main()
        {
            texCoordVarying = gl_MultiTexCoord0.xy;
            gl_Position = ftransform();
        }
        )GLSL";
        
        fragment = R"GLSL(
        #version 120
        uniform sampler2DRect tex0;
        uniform float threshold;
		uniform float gamma;
		uniform bool useHSV;
		uniform vec3 hsvRange;
        uniform vec3 chromaKeyColor;
        varying vec2 texCoordVarying;

		vec3 rgb2hsv(vec3 c){
			vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
			vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
			vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

			float d = q.x - min(q.w, q.y);
			float e = 1.0e-10;
			return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
		}

		void main()
        {
            vec4 texel0 = texture2DRect(tex0, texCoordVarying);

			if(useHSV){
				vec3 hsvTexel = rgb2hsv(texel0.rgb);
				vec3 hsvKey = rgb2hsv(chromaKeyColor.rgb);
				//float diff = length(hsvKey - hsvTexel);
				float diffHue = min(abs(hsvKey.r - hsvTexel.r), 1 - abs(hsvKey.r - hsvTexel.r)) / 0.5;
				float diffSat = abs(hsvKey.g - hsvTexel.g);
				float diffVal = abs(hsvKey.b - hsvTexel.b);

				float diff = (diffHue * hsvRange.r + diffSat * hsvRange.g + diffVal * hsvRange.b) / (hsvRange.r + hsvRange.g + hsvRange.b);
				//float diff = diffHue;

				if(diff < threshold){
					gl_FragColor = vec4(texel0.rgb, pow(diff / threshold, gamma));
				}else{
					gl_FragColor = texel0;
				}

				//gl_FragColor = vec4(diffHue, diffHue, diffHue, 1.0);

			}else{
				float diff = length(chromaKeyColor - texel0.rgb);
				if(diff < threshold){
					gl_FragColor = vec4(texel0.rgb, pow(diff / threshold, gamma));
				}else{
					gl_FragColor = texel0;
				}
			}
        }
        )GLSL";
    }

    shader.setupShaderFromSource(GL_VERTEX_SHADER, vertex);
    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, fragment);
    shader.bindDefaults();
    shader.linkProgram();
}

void ofxChromaKey::begin(){
    shader.begin();
    shader.setUniform3f("chromaKeyColor", ofVec3f(keyColor.r/255.0, keyColor.g/255.0, keyColor.b/255.0));
    shader.setUniform1f("threshold", threshold);
	shader.setUniform1f("gamma", gamma);
	shader.setUniform3f("hsvRange", ofVec3f(hueRange, satRange, valRange));
	shader.setUniform1i("useHSV", useHSV);

}

void ofxChromaKey::end(){
    shader.end();
}
