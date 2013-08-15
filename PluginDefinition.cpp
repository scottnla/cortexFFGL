#include <FFGL.h>
#include <FFGLLib.h>

#include "PluginDefinition.h"

#include <fstream>
#include <cmath>
#include <sys/time.h>


Parameter::Parameter(string name, float min, float max, float value)
: Name(name)
, RangeMin(min)
, RangeMax(max)
, UniformLocation(-1)
{
    Value = (value - min) / (max - min);
}

float Parameter::GetScaledValue() const {
    return RangeMin + Value * (RangeMax - RangeMin);
}


static CFFGLPluginInfo PluginInfo ( 
	ShaderPlugin::CreateInstance,	// Create method
	"GMAR",								// Plugin unique ID
	"Grainmarch",					// Plugin name
	1,						   			// API major version number
	500,								  // API minor version number
	1,										// Plugin major version number
	000,									// Plugin minor version number
	FF_SOURCE,						// Plugin type
	"Grainy fractal raymarching",	// Plugin description
	"by Michael Dewberry, based on Shadertoy shaders by Syntopia and " // About
);


#include "shader_vert.glsl"
#include "shader_frag.glsl"


ShaderPlugin::ShaderPlugin()
: CFreeFrameGLPlugin()
, m_initResources(1)
{
	SetMinInputs(0);
	SetMaxInputs(0);
    SetTimeSupported(true);
    m_HostSupportsSetTime = false;
    
    m_time = 0;
    m_timeLocation = -1;
    
    m_resolution[0] = 0;
    m_resolution[1] = 0;
    m_resolution[2] = 0;
    m_resolutionLocation = -1;

    m_parameters.push_back(Parameter("FieldOfView", 0.1, 10.0, 1.0));

    for (int ii = 0; ii < m_parameters.size(); ii++) {
        auto p = m_parameters[ii];
        SetParamInfo(ii, p.Name.c_str(), FF_TYPE_STANDARD, p.Value);
    }
}

DWORD ShaderPlugin::InitGL(const FFGLViewportStruct *vp)
{
    m_extensions.Initialize();
    if (m_extensions.multitexture==0 || m_extensions.ARB_shader_objects==0)
        return FF_FAIL;
    
    m_shader.SetExtensions(&m_extensions);
    m_shader.Compile(vertexShaderCode, fragmentShaderCode);
 
    m_shader.BindShader();
    
    for (auto p : m_parameters) {
        p.UniformLocation = m_shader.FindUniform(p.Name.c_str());
    }
    
    m_timeLocation = m_shader.FindUniform("iGlobalTime");
    m_resolutionLocation = m_shader.FindUniform("iResolution");
    m_resolution[0] = vp->width;
    m_resolution[1] = vp->height;
    
    m_shader.UnbindShader();
    
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   
    return FF_SUCCESS;
}

DWORD ShaderPlugin::DeInitGL()
{
  m_shader.FreeGLResources();

  return FF_SUCCESS;
}

DWORD ShaderPlugin::ProcessOpenGL(ProcessOpenGLStruct *pGL) {
    
    m_shader.BindShader();
    
    for (auto p : m_parameters) {
        m_extensions.glUniform1fARB(p.UniformLocation, p.Value);
    }
    
    if (!m_HostSupportsSetTime)
    {
        update_time(&m_time, m_startTime);
    }
    
    m_extensions.glUniform1fARB(m_timeLocation, m_time);
    m_extensions.glUniform3fvARB(m_resolutionLocation, 3, m_resolution);
        
	glBegin(GL_QUADS);
    m_extensions.glMultiTexCoord2f(GL_TEXTURE0, 0, 0);
	glVertex3f(-1,-1,1);
    m_extensions.glMultiTexCoord2f(GL_TEXTURE0, 0, m_resolution[1]);
	glVertex3f(-1,1,1);
    m_extensions.glMultiTexCoord2f(GL_TEXTURE0, m_resolution[0], m_resolution[1]);
	glVertex3f(1,1,1);
    m_extensions.glMultiTexCoord2f(GL_TEXTURE0, m_resolution[0], 0);
	glVertex3f(1,-1,1);
	glEnd();
  
    m_shader.UnbindShader();
    
    return FF_SUCCESS;
}

DWORD ShaderPlugin::GetParameter(DWORD dwIndex)
{
	DWORD dwRet;

    if (dwIndex < m_parameters.size()) {
        auto p = m_parameters[dwIndex];
        *((float *)(unsigned)&dwRet) = p.Value;
        return dwRet;
    } else {
        return FF_FAIL;
	}
}

DWORD ShaderPlugin::SetParameter(const SetParameterStruct* pParam)
{
    if (pParam != NULL && pParam->ParameterNumber < m_parameters.size()) {
        auto p = m_parameters[pParam->ParameterNumber];
        p.Value = *((float *)(unsigned)&(pParam->NewParameterValue));
        return FF_SUCCESS;
    } else {
        return FF_FAIL;
    }
}

DWORD ShaderPlugin::SetTime(double time)
{
    m_HostSupportsSetTime = true;
    m_time = time;
    return FF_SUCCESS;
}

void update_time(double *t, const double t0)
{
#ifdef _WIN32
    //amount of time since plugin init, in seconds (same as SetTime):
    *t = double(GetTickCount())/1000.0 - t0;
#else
    timeval time;
    gettimeofday(&time, NULL);
    long millis = (time.tv_sec * 1000) + (time.tv_usec / 1000);
    *t = double(millis)/1000.0f - t0;
#endif
    return;
}

