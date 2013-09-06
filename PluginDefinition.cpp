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
                                   "CORTEX",								// Plugin unique ID
                                   "cortex",					// Plugin name
                                   1,						   			// API major version number
                                   500,								  // API minor version number
                                   0,										// Plugin major version number
                                   002,									// Plugin minor version number
                                   FF_SOURCE,						// Plugin type
                                   "Visual form constant synthesizer",	// Plugin description
                                   "by Nathan Lachenmyer (morphogen)" // About
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

    // To FFGLize an arbitrary shader, replace shader_frag.glsl and the parameters below.
    // Parameter(uniform name, minimum value, maximum value, default value)
    // Standard parameters/float uniforms only for now.
    
    m_parameters.push_back(Parameter("sVert", 0.0, 1.0, 0.0));
    m_parameters.push_back(Parameter("sHorizon", 0.0, 1.0, 0.0));
    m_parameters.push_back(Parameter("sDiag", 0.0, 1.0, 0.0));
    m_parameters.push_back(Parameter("sDiagAlt", 0.0, 1.0, 0.0));
    m_parameters.push_back(Parameter("sArms", 0.0, 1.0, 0.0));
    m_parameters.push_back(Parameter("sRings", 0.0, 1.0, 1.0));
    m_parameters.push_back(Parameter("sSpiral", 0.0, 1.0, 0.0));
    m_parameters.push_back(Parameter("sSpiralAlt", 0.0, 1.0, 0.0));
    m_parameters.push_back(Parameter("vertPeriod", -20.0, 10.0, 4.0));
    m_parameters.push_back(Parameter("horizonPeriod", -20.0, 20.0, 4.0));
    m_parameters.push_back(Parameter("diagPeriod", -20.0, 20.0, 4.0));
    m_parameters.push_back(Parameter("diagAltPeriod", -20.0, 20.0, 4.0));
    m_parameters.push_back(Parameter("armPeriod", -20.0, 20.0, 4.0));
    m_parameters.push_back(Parameter("ringPeriod", -20.0, 20.0, 4.0));
    m_parameters.push_back(Parameter("spiralPeriod", -20.0, 20.0, 4.0));
    m_parameters.push_back(Parameter("spiralAltPeriod", -20.0, 20.0, 4.0));
    m_parameters.push_back(Parameter("numVert", 0.0, 100.0, 40.0));
    m_parameters.push_back(Parameter("numHorizon", 0.0, 100.0, 40.0));
    m_parameters.push_back(Parameter("numDiag", 0.0, 100.0, 40.0));
    m_parameters.push_back(Parameter("numDiagAlt", 0.0, 100.0, 40.0));
    m_parameters.push_back(Parameter("numArms", 0.0, 24.0, 4.0));
    m_parameters.push_back(Parameter("numRings", 0.0, 24.0, 4.0));
    m_parameters.push_back(Parameter("numSpiral", 0.0, 24.0, 4.0));
    m_parameters.push_back(Parameter("numSpiralAlt", 0.0, 24.0, 4.0));
    
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
    
    for (auto& p : m_parameters) {
        p.UniformLocation = m_shader.FindUniform(p.Name.c_str());
        if (p.UniformLocation < 0) {
            fprintf(stderr, "Could not locate uniform %s in shader!", p.Name.c_str());
        }
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
    
    for (auto& p : m_parameters) {
        m_extensions.glUniform1fARB(p.UniformLocation, p.GetScaledValue());
    }
    
    if (!m_HostSupportsSetTime)
    {
        update_time(&m_time, m_startTime);
    }
    
    m_extensions.glUniform1fARB(m_timeLocation, m_time);
    m_extensions.glUniform3fvARB(m_resolutionLocation, 3, m_resolution);
        
	glBegin(GL_QUADS);
    //m_extensions.glMultiTexCoord2f(GL_TEXTURE0, 0, 0);
	glVertex4f(-1, -1, 0, 1);
    //m_extensions.glMultiTexCoord2f(GL_TEXTURE0, 0, m_resolution[1]);
	glVertex4f(-1, 1, 0, 1);
    //m_extensions.glMultiTexCoord2f(GL_TEXTURE0, m_resolution[0], m_resolution[1]);
	glVertex4f(1, 1, 0, 1);
    //m_extensions.glMultiTexCoord2f(GL_TEXTURE0, m_resolution[0], 0);
	glVertex4f(1, -1, 0, 1);
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
        auto& p = m_parameters[pParam->ParameterNumber];
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

