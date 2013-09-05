#pragma once

#include <FFGLShader.h>
#include <FFGLPluginSDK.h>
#include <string>
#include <vector>

using namespace std;

class Parameter {
public:
    Parameter(string name, float min, float max, float value);
    float GetScaledValue() const;
    
    string Name;
    float Value;
    GLint UniformLocation;
    float RangeMin;
    float RangeMax;
};

class ShaderPlugin :
public CFreeFrameGLPlugin
{
public:
    ShaderPlugin();
    virtual ~ShaderPlugin() {}

	DWORD SetParameter(const SetParameterStruct* pParam);
	DWORD GetParameter(DWORD dwIndex);
	DWORD ProcessOpenGL(ProcessOpenGLStruct* pGL);
    DWORD InitGL(const FFGLViewportStruct *vp);
    DWORD DeInitGL();
    DWORD SetTime(double time);

	static DWORD __stdcall CreateInstance(CFreeFrameGLPlugin **ppOutInstance)
    {
        *ppOutInstance = new ShaderPlugin();
        if (*ppOutInstance != NULL)
            return FF_SUCCESS;
        return FF_FAIL;
    }

protected:	
	int m_initResources;
	FFGLExtensions m_extensions;
    FFGLShader m_shader;
    
    vector<Parameter> m_parameters;

    double m_startTime;
    double m_time;
    GLint m_timeLocation;
    float m_resolution[3];
    GLint m_resolutionLocation;
    
    bool m_HostSupportsSetTime;
};

void update_time(double *t, const double t0);
