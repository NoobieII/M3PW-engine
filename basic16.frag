#version 120

varying vec4 position_from_vert;
varying vec2 uv_from_vert;
varying float tid_from_vert;
varying vec4 color_from_vert;

varying float intensity_from_vert;

uniform sampler2D textures[32];

#define tiv tid_from_vert
#define uvv uv_from_vert

void main()
{
	vec4 texcolor = vec4(1, 1, 1, 1);
	
	if(tiv > 0.0){
		int tid = int(tiv - 0.5);
		
		if(tid < 16){
			if(tid < 8){
				if(tid < 4){
					if(tid < 2){
						if(tid == 0) texcolor = texture2D(textures[0], uvv);
						else         texcolor = texture2D(textures[1], uvv);
					}
					else{
						if(tid == 2) texcolor = texture2D(textures[2], uvv);
						else         texcolor = texture2D(textures[3], uvv);
					}
				}
				else{
					if(tid < 6){
						if(tid == 4) texcolor = texture2D(textures[4], uvv);
						else         texcolor = texture2D(textures[5], uvv);
					}
					else{
						if(tid == 6) texcolor = texture2D(textures[6], uvv);
						else         texcolor = texture2D(textures[7], uvv);
					}
				}
			}
			else{
				if(tid < 12){
					if(tid < 10){
						if(tid == 8) texcolor = texture2D(textures[8], uvv);
						else         texcolor = texture2D(textures[9], uvv);
					}
					else{
						if(tid == 10) texcolor = texture2D(textures[10], uvv);
						else          texcolor = texture2D(textures[11], uvv);
					}
				}
				else{
					if(tid < 14){
						if(tid == 12) texcolor = texture2D(textures[12], uvv);
						else          texcolor = texture2D(textures[13], uvv);
					}
					else{
						if(tid == 14) texcolor = texture2D(textures[14], uvv);
						else          texcolor = texture2D(textures[15], uvv);
					}
				}
			}
		}
		/*
		else{
			if(tid < 24){
				if(tid < 20){
					if(tid < 18){
						if(tid == 16) texcolor = texture2D(textures[16], uvv);
						else          texcolor = texture2D(textures[17], uvv);
					}
					else{
						if(tid == 18) texcolor = texture2D(textures[18], uvv);
						else          texcolor = texture2D(textures[19], uvv);
					}
				}
				else{
					if(tid < 22){
						if(tid == 20) texcolor = texture2D(textures[20], uvv);
						else          texcolor = texture2D(textures[21], uvv);
					}
					else{
						if(tid == 22) texcolor = texture2D(textures[22], uvv);
						else          texcolor = texture2D(textures[23], uvv);
					}
				}
			}
			else{
				if(tid < 28){
					if(tid < 26){
						if(tid == 24) texcolor = texture2D(textures[24], uvv);
						else          texcolor = texture2D(textures[25], uvv);
					}
					else{
						if(tid == 26) texcolor = texture2D(textures[26], uvv);
						else          texcolor = texture2D(textures[27], uvv);
					}
				}
				else{
					if(tid < 30){
						if(tid == 28) texcolor = texture2D(textures[28], uvv);
						else          texcolor = texture2D(textures[29], uvv);
					}
					else{
						if(tid == 30) texcolor = texture2D(textures[30], uvv);
						else          texcolor = texture2D(textures[31], uvv);
					}
				}
			}
		}
		*/
	}
	
	gl_FragColor = vec4(texcolor.xyz * intensity_from_vert, texcolor.w) * color_from_vert;
}

