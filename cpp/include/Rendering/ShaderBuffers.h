#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#define GLSL_DEFINITION(name) uniform name
#define F_DEFINITION(name) struct name

#define GLSL_MAT4(name) mat4 name;
#define F_MAT4(name) alignas(16) glm::mat4 name;

#define GLSL_VEC2(name) vec2 name;
#define F_VEC2(name) alignas(16) glm::vec2 name;

#define SHADER_UNIFORM_STR(S) #S
#define GLSL_UNIFORM_STRING(location, name, structure) "layout(binding=" + (location) + ") " SHADER_UNIFORM_STR(structure) " " + (name) + ";" 
#define GLSL_PUSHBUFFER_STRING(name, structure) "layout(push_constant) " SHADER_UNIFORM_STR(structure) " " + (name) + ";"

#define CAMERA_SHADER_STRUCTURE(D, M4) \
D(CameraShaderBuffer) \
{ \
M4(View) \
M4(Proj) \
M4(InvView) \
M4(InvProj) \
M4(ViewProj) \
}
#define GLSL_CAMERA_SHADER_STRUCTURE CAMERA_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_MAT4)

#define MODEL_SHADER_STRUCTURE(D, M4) \
D(ModelShaderBuffer) \
{ \
M4(Model) \
M4(InvModel) \
}
#define GLSL_MODEL_SHADER_STRUCTURE MODEL_SHADER_STRUCTURE(GLSL_DEFINITION, GLSL_MAT4)

#define TIME_SHADER_BUFFER(D, V2) \
D(TimeShaderBuffer) \
{ \
V2(Time) \
}
#define GLSL_TIME_SHADER_STRUCTURE TIME_SHADER_BUFFER(GLSL_DEFINITION, GLSL_VEC2)

CAMERA_SHADER_STRUCTURE(F_DEFINITION, F_MAT4);
MODEL_SHADER_STRUCTURE(F_DEFINITION, F_MAT4);
TIME_SHADER_BUFFER(F_DEFINITION, F_VEC2);