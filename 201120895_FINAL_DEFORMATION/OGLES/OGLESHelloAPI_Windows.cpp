#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <vector>
#include "image.h"
#include "normalMap.h"
#define	WINDOW_CLASS_NAME _T("PVRShellClass")
#define APPLICATION_NAME _T("HelloAPI")
#define ERROR_TITLE _T("Error")

#define LENTRANSROW	4
#define COORD 1.0f

const unsigned int WindowWidth = 1000;
const unsigned int WindowHeight = 800;

const unsigned int VertexArray = 0;
const unsigned int textureCoord = 1;
const unsigned int NormalArray = 2;
const unsigned int NormalMap = 3;

GLuint textureId[2];
static const GLubyte pixels[4 * 4] = {
	255, 150, 0, 255,
	0, 255, 0, 255,
	0, 0, 255, 255,
	255, 255, 0, 255
};
bool HasUserQuit = false;

float length(const float x, const float y, const float z) {
	return (float)sqrt(x*x + y*y + z*z);
}
class mat4 {
private:
	float mat4TransformationMatrix[LENTRANSROW*LENTRANSROW] = { 0, };

public:
	float* getTransformationMatrix(void);
	void setTransformationMatrix(float* matrix);
	void setTransformationMatrix(int indexOfMatrix, float value);
	void ajouLoadIdentity(void);
	void ajouLoadMatrix(mat4 matrix);
	void ajouMultMatrix(mat4 m);
	void ajouTranslate3f(float x, float y, float z);
	void ajouTranslate3f_xyz(float x, float y, float z);
	void ajouScale3f(float x, float y, float z);
	void ajouRotateX1f(float theta);
	void ajouRotateY1f(float theta);
	void ajouRotateZ1f(float theta);
	void viewSetLookat(const float eyex, const float eyey, const float eyez,
		const float atx, const float aty, const float atz,
		const float upx, const float upy, const float upz);
	void projLookat(const float xmin, const float xmax, const float ymin, const float ymax, const float zmin, const float zmax);
};
float* mat4::getTransformationMatrix() {
	return mat4TransformationMatrix;
}
void mat4::setTransformationMatrix(float* matrix) {
	for (int index = 0; index < LENTRANSROW*LENTRANSROW; index++)
		mat4TransformationMatrix[index] = matrix[index];
	return;
}
void mat4::setTransformationMatrix(int indexOfMatrix, float value)
{
	mat4TransformationMatrix[indexOfMatrix] = value;
}
void mat4::ajouLoadIdentity(void)
{
	for (int index = 0; index < LENTRANSROW*LENTRANSROW; index++)
		mat4TransformationMatrix[index] = 0;

	mat4TransformationMatrix[0] = mat4TransformationMatrix[5] = mat4TransformationMatrix[10] = mat4TransformationMatrix[15] = 1;

	return;
}
void mat4::ajouLoadMatrix(mat4 m)
{
	setTransformationMatrix(m.getTransformationMatrix());
	return;
}
void mat4::ajouMultMatrix(mat4 m)
{
	float tmpMatrix[LENTRANSROW*LENTRANSROW] = { 0, };

	//ajouLoadIdentity();

	for (int index = 0; index < 16; index++)
		for (int indexC = index % 4; indexC < 16; indexC = indexC + 4)
			tmpMatrix[index] += mat4TransformationMatrix[index / 4 * 4 + indexC / 4] * m.getTransformationMatrix()[indexC];

	/*
	// Column major?
	for (int index = 0; index < 16; index++)
	for (int indexC = index / 4 * 4; indexC < index / 4 *4 + 4; indexC++)
	tmpMatrix[index] += mat4TransformationMatrix[index] * m.getTransformationMatrix()[indexC];
	*/

	setTransformationMatrix(tmpMatrix);

	return;
}
void mat4::ajouTranslate3f(float x, float y, float z)
{
	mat4TransformationMatrix[3] += x;
	mat4TransformationMatrix[7] += y;
	mat4TransformationMatrix[11] += z;
	mat4TransformationMatrix[15] = 1;
}
void mat4::ajouTranslate3f_xyz(float x, float y, float z)
{
	mat4TransformationMatrix[12] += mat4TransformationMatrix[0] * x + mat4TransformationMatrix[4] * y + mat4TransformationMatrix[8] * z;
	mat4TransformationMatrix[13] += mat4TransformationMatrix[1] * x + mat4TransformationMatrix[5] * y + mat4TransformationMatrix[9] * z;
	mat4TransformationMatrix[14] += mat4TransformationMatrix[2] * x + mat4TransformationMatrix[6] * y + mat4TransformationMatrix[10] * z;
	mat4TransformationMatrix[15] += mat4TransformationMatrix[3] * x + mat4TransformationMatrix[7] * y + mat4TransformationMatrix[11] * z;
}
void mat4::ajouScale3f(float x, float y, float z)
{
	mat4TransformationMatrix[0] = mat4TransformationMatrix[0] * x;
	mat4TransformationMatrix[5] = mat4TransformationMatrix[5] * y;
	mat4TransformationMatrix[10] = mat4TransformationMatrix[10] * z;
}
void mat4::ajouRotateX1f(float theta)
{
	mat4 rotateMatrix;

	rotateMatrix.setTransformationMatrix(0, 1);
	rotateMatrix.setTransformationMatrix(15, 1);
	rotateMatrix.setTransformationMatrix(5, cos(theta));
	rotateMatrix.setTransformationMatrix(10, cos(theta));
	rotateMatrix.setTransformationMatrix(6, -sin(theta));
	rotateMatrix.setTransformationMatrix(9, sin(theta));

	ajouMultMatrix(rotateMatrix);
}
void mat4::ajouRotateY1f(float theta)
{
	mat4 rotateMatrix;

	rotateMatrix.setTransformationMatrix(5, 1);
	rotateMatrix.setTransformationMatrix(15, 1);
	rotateMatrix.setTransformationMatrix(0, cos(theta));
	rotateMatrix.setTransformationMatrix(10, cos(theta));
	rotateMatrix.setTransformationMatrix(8, -sin(theta));
	rotateMatrix.setTransformationMatrix(2, sin(theta));

	ajouMultMatrix(rotateMatrix);
}
void mat4::ajouRotateZ1f(float theta)
{
	mat4 rotateMatrix;

	rotateMatrix.setTransformationMatrix(10, 1);
	rotateMatrix.setTransformationMatrix(15, 1);
	rotateMatrix.setTransformationMatrix(0, cos(theta));
	rotateMatrix.setTransformationMatrix(5, cos(theta));
	rotateMatrix.setTransformationMatrix(1, -sin(theta));
	rotateMatrix.setTransformationMatrix(4, sin(theta));

	ajouMultMatrix(rotateMatrix);
}
void mat4::viewSetLookat(const float eyex, const float eyey, const float eyez,
	const float atx, const float aty, const float atz,
	const float upx, const float upy, const float upz) {
	float p[3] = { 0, };
	float n[3] = { 0, };
	float u[3] = { 0, };
	float v[3] = { 0, };

	p[0] = eyex; p[1] = eyey; p[2] = eyez;
	n[0] = atx - eyex; n[1] = aty - eyey; n[2] = atz - eyez;
	float I = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
	n[0] /= I;
	n[1] /= I;
	n[2] /= I;

	u[0] = upy * n[2] - upz * n[1];
	u[1] = upz * n[0] - upx * n[2];
	u[2] = upx * n[1] - upy * n[0];
	I = sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
	u[0] /= I;
	u[1] /= I;
	u[2] /= I;

	v[0] = n[1] * u[2] - n[2] * u[1];
	v[1] = n[2] * u[0] - n[0] * u[2];
	v[2] = n[0] * u[1] - n[1] * u[0];

	mat4TransformationMatrix[0] = u[0];
	mat4TransformationMatrix[1] = v[0];
	mat4TransformationMatrix[2] = n[0];
	mat4TransformationMatrix[3] = 0.0f;

	mat4TransformationMatrix[4] = u[1];
	mat4TransformationMatrix[5] = v[1];
	mat4TransformationMatrix[6] = n[1];
	mat4TransformationMatrix[7] = 0.0f;

	mat4TransformationMatrix[8] = u[2];
	mat4TransformationMatrix[9] = v[2];
	mat4TransformationMatrix[10] = n[2];
	mat4TransformationMatrix[11] = 0.0f;

	mat4TransformationMatrix[12] = 0.0f;
	mat4TransformationMatrix[13] = 0.0f;
	mat4TransformationMatrix[14] = 0.0f;
	mat4TransformationMatrix[15] = 1.0f;

	// Load TransformationMatrix
	static float LoadTransformationMatrix[] =
	{
		\
		1.0f, 0.0f, 0.0f, -p[0],
		0.0f, 1.0f, 0.0f, -p[1],
		0.0f, 0.0f, 1.0f, -p[2],
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	// View TransformationMatrix

	mat4 temp;
	temp.setTransformationMatrix(LoadTransformationMatrix);

	ajouMultMatrix(temp);
}

void mat4::projLookat(const float xmin, const float xmax, const float ymin, const float ymax, const float zmin, const float zmax) {
	mat4TransformationMatrix[0] = 2.0f / (xmax - xmin);
	mat4TransformationMatrix[1] = 0.0f;
	mat4TransformationMatrix[2] = 0.0f;
	mat4TransformationMatrix[3] = 0.0f;
	mat4TransformationMatrix[4] = 0.0f;
	mat4TransformationMatrix[5] = 2.0f / (ymax - ymin);
	mat4TransformationMatrix[6] = 0.0f;
	mat4TransformationMatrix[7] = 0.0f;
	mat4TransformationMatrix[8] = 0.0f;
	mat4TransformationMatrix[9] = 0.0f;
	mat4TransformationMatrix[10] = 2.0f / (zmax - zmin);
	mat4TransformationMatrix[11] = 0.0f;
	mat4TransformationMatrix[12] = -(xmax + xmin) / (xmax - xmin);
	mat4TransformationMatrix[13] = -(ymax + ymin) / (ymax - ymin);
	mat4TransformationMatrix[14] = -(zmax + zmin) / (zmax - zmin);
	mat4TransformationMatrix[15] = 1.0f;
}

class Light
{
public:
	Light();
	~Light();

	GLfloat pos[4];
	GLfloat amb[4];
	GLfloat diff[4];
	GLfloat spec[4];
	GLfloat att[4];
	GLfloat color[3];
};

Light::Light()
{
	pos[0] = 1.5f; pos[1] = 1.5f; pos[2] = 1.0f; pos[3] = 1.0f;
	amb[0] = 0.0f; amb[1] = 0.0f; amb[2] = 0.0f; amb[3] = 1.0f;
	diff[0] = 1.0f; diff[1] = 1.0f; diff[2] = 1.0f; diff[3] = 1.0f;
	spec[0] = 1.0f; spec[1] = 1.0f; spec[2] = 1.0f; spec[3] = 1.0f;
	att[0] = 1.0f; att[1] = 0.0f; att[2] = 0.0f; att[3] = 1.0f;
	color[0] = 1.0f, color[1] = 1.0f, color[2] = 1.0f;
}

Light::~Light()
{
}


class Material
{
public:
	Material();
	~Material();

	GLfloat amb[4];
	GLfloat diff[4];
	GLfloat spec[4];
	float shine;
};

Material::Material()
{
	amb[0] = 0.2f; amb[1] = 0.2f; amb[2] = 1.5f; amb[3] = 1.0f;
	diff[0] = 0.0f; diff[1] = 0.0f; diff[2] = 0.0f; diff[3] = 1.0f;
	spec[0] = 1.0f; spec[1] = 1.0f; spec[2] = 1.0f; spec[3] = 1.0f;
	shine = 1.0f;
}

Material::~Material()
{
}

LRESULT CALLBACK handleWindowMessages(HWND nativeWindow, UINT message, WPARAM windowParameters, LPARAM longWindowParameters)
{
	switch (message)
	{
	case WM_SYSCOMMAND:
	{
		switch (windowParameters)
		{
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
		{
			return 0;
		}
		}
		break;
	}
	case WM_CLOSE:
	{
		HasUserQuit = true;
		PostQuitMessage(0);
		return 1;
	}
	}

	return DefWindowProc(nativeWindow, message, windowParameters, longWindowParameters);
}

bool testEGLError(HWND nativeWindow, const char* functionLastCalled)
{
	EGLint lastError = eglGetError();
	if (lastError != EGL_SUCCESS)
	{
		TCHAR stringBuffer[256];
		_stprintf(stringBuffer, _T("%s failed (%x).\n"), functionLastCalled, lastError);
		MessageBox(nativeWindow, stringBuffer, ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	return true;
}

bool testGLError(HWND nativeWindow, const char* functionLastCalled)
{
	GLenum lastError = glGetError();
	if (lastError != GL_NO_ERROR)
	{
		TCHAR stringBuffer[256];
		_stprintf(stringBuffer, _T("%s failed (%x).\n"), functionLastCalled, lastError);
		MessageBox(nativeWindow, stringBuffer, ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

bool createWindowAndDisplay(HINSTANCE applicationInstance, HWND& nativeWindow, HDC& deviceContext)
{
	// Describe the native window in a window class structure
	WNDCLASS nativeWindowDescription;
	nativeWindowDescription.style = CS_HREDRAW | CS_VREDRAW;
	nativeWindowDescription.lpfnWndProc = handleWindowMessages;
	nativeWindowDescription.cbClsExtra = 0;
	nativeWindowDescription.cbWndExtra = 0;
	nativeWindowDescription.hInstance = applicationInstance;
	nativeWindowDescription.hIcon = 0;
	nativeWindowDescription.hCursor = 0;
	nativeWindowDescription.lpszMenuName = 0;
	nativeWindowDescription.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	nativeWindowDescription.lpszClassName = WINDOW_CLASS_NAME;

	// Register the windows class with the OS.
	ATOM registerClass = RegisterClass(&nativeWindowDescription);
	if (!registerClass)
	{
		MessageBox(0, _T("Failed to register the window class"), ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);
	}

	// Create a rectangle describing the area of the window
	RECT windowRectangle;
	SetRect(&windowRectangle, 0, 0, WindowWidth, WindowHeight);
	AdjustWindowRectEx(&windowRectangle, WS_CAPTION | WS_SYSMENU, false, 0);

	// Create the window from the available information
	nativeWindow = CreateWindow(WINDOW_CLASS_NAME, APPLICATION_NAME, WS_VISIBLE | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT,
		windowRectangle.right - windowRectangle.left, windowRectangle.bottom - windowRectangle.top,
		NULL, NULL, applicationInstance, NULL);
	if (!nativeWindow)
	{
		MessageBox(0, _T("Failed to create the window"), ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	// Get the associated device context from the window
	deviceContext = GetDC(nativeWindow);
	if (!deviceContext)
	{
		MessageBox(nativeWindow, _T("Failed to create the device context"), ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

bool createEGLDisplay(HDC deviceContext, EGLDisplay& eglDisplay)
{
	if (eglDisplay == EGL_NO_DISPLAY)
	{
		eglDisplay = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
	}

	// If a display still couldn't be obtained, return an error.
	if (eglDisplay == EGL_NO_DISPLAY)
	{
		MessageBox(0, _T("Failed to get an EGLDisplay"), ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	EGLint eglMajorVersion, eglMinorVersion;
	if (!eglInitialize(eglDisplay, &eglMajorVersion, &eglMinorVersion))
	{
		MessageBox(0, _T("Failed to initialize the EGLDisplay"), ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

bool chooseEGLConfig(EGLDisplay eglDisplay, EGLConfig& eglConfig)
{

	const EGLint configurationAttributes[] =
	{
		EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	EGLint configsReturned;
	if (!eglChooseConfig(eglDisplay, configurationAttributes, &eglConfig, 1, &configsReturned) || (configsReturned != 1))
	{
		MessageBox(0, _T("eglChooseConfig() failed."), ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

bool createEGLSurface(HWND nativeWindow, EGLDisplay eglDisplay, EGLConfig eglConfig, EGLSurface& eglSurface)
{
	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, NULL);
	if (eglSurface == EGL_NO_SURFACE)
	{
		eglGetError(); // Clear error
		eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, NULL, NULL);
	}

	// Check for any EGL Errors
	if (!testEGLError(nativeWindow, "eglCreateWindowSurface")) { return false; }
	return true;
}

bool setupEGLContext(EGLDisplay eglDisplay, EGLConfig eglConfig, EGLSurface eglSurface, EGLContext& eglContext, HWND nativeWindow)
{
	eglBindAPI(EGL_OPENGL_ES_API);
	if (!testEGLError(nativeWindow, "eglBindAPI")) { return false; }

	EGLint contextAttributes[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, contextAttributes);
	if (!testEGLError(nativeWindow, "eglCreateContext")) { return false; }

	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	if (!testEGLError(nativeWindow, "eglMakeCurrent")) { return false; }
	return true;
}

bool initializeBuffer(GLuint& vertexBuffer, GLuint& texBuffer, HWND nativeWindow)
{
	static const GLfloat vertexData[] = {
		// BOTTOM 
		// 2 6 4 
		COORD, COORD, -COORD,	1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		-COORD, -COORD, -COORD, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,
		-COORD, COORD, -COORD,	0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		// 2 8 6
		COORD, COORD, -COORD,	1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
		COORD, -COORD, -COORD,	1.0f, 1.0f, 0.0f, 0.0f, -1.0f,
		-COORD, -COORD, -COORD, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f,

		// SIDE 
		// 1 2 4
		COORD, COORD, COORD,	0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		COORD, COORD, -COORD,	0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		-COORD, COORD, -COORD,	1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		// 1 4 3
		COORD, COORD, COORD,	0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		-COORD, COORD, -COORD,	1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		-COORD, COORD, COORD,	1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		// 6 5 4
		-COORD, -COORD, -COORD, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-COORD, -COORD, COORD,	1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
		-COORD, COORD, -COORD,	0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		// 3 4 5
		-COORD, COORD, COORD,	0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
		-COORD, COORD, -COORD,	0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		-COORD, -COORD, COORD,	1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
		// 7 5 6
		COORD, -COORD, COORD,	1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
		-COORD, -COORD, COORD,	0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
		-COORD, -COORD, -COORD, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		// 7 6 8
		COORD, -COORD, COORD,	1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
		-COORD, -COORD, -COORD, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		COORD, -COORD, -COORD,	1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
		// 1 8 2
		COORD, COORD, COORD,	1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		COORD, -COORD, -COORD,	0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		COORD, COORD, -COORD,	1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		// 8 1 7
		COORD, -COORD, -COORD,	0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		COORD, COORD, COORD,	1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		COORD, -COORD, COORD,	0.0f, 1.0f, 1.0f, 0.0f, 0.0f,

		// TOP 
		// 3 5 7
		-COORD, COORD, COORD,	1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-COORD, -COORD, COORD,	1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		COORD, -COORD, COORD,	0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		// 1 3 7
		COORD, COORD, COORD,	0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-COORD, COORD, COORD,	1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		COORD, -COORD, COORD,	0.0f, 1.0f, 0.0f, 0.0f, 1.0f
	};

	// Generate a buffer object
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glEnable(GL_DEPTH_TEST);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(2, textureId);


	if (!testGLError(nativeWindow, "glBufferData")) { return false; }


	return true;
}

bool initializeShaders(GLuint& fragmentShader, GLuint& vertexShader, GLuint& shaderProgram, HWND nativeWindow)
{
	const char* const fragmentShaderSource = "\
                                                precision highp float;\
                                                struct LightSource {\
                                                    vec4 pos;\
                                                    vec4 amb;\
                                                    vec4 diff;\
                                                    vec4 spec;\
                                                    vec4 att;\
                                                    vec3 color;\
                                                };\
                                                uniform LightSource light;\
                                                \
                                                struct Material {\
                                                    vec4 amb;\
                                                    vec4 diff;\
                                                    vec4 spec;\
                                                    float shine;\
                                                };\
                                                uniform Material material;\
                                                \
                                                varying vec4 vPosition;\
                                                varying vec4 vNormal;\
                                                varying highp vec2 outCoord;\
                                                uniform sampler2D s_texture;\
                                                uniform sampler2D n_texture;\
                                                \
                                                uniform vec4 light_color;\
                                                \
                                                const float specularScale = 0.65;\
                                                const float shininess = 20.0;\
                                                const float roughness = 1.0;\
                                                const float albedo = 0.95;\
                                                const float gamma = 2.2;\
                                                const float PI = 3.141592;\
                                                \
                                                \
                                                float toLinear(float v){\
                                                    return pow(v, gamma);\
                                                }\
                                                vec2 toLinear(vec2 v){\
                                                    return pow(v, vec2(gamma));\
                                                }\
                                                vec3 toLinear(vec3 v){\
                                                    return pow(v, vec3(gamma));\
                                                }\
                                                vec4 toLinear(vec4 v){\
                                                    return vec4(toLinear(v.rgb), v.a);\
                                                }\
                                                \
                                                vec4 textureLinear(sampler2D uTex, vec2 uv) {\
                                                    return toLinear(texture2D(uTex, uv));\
                                                }\
                                                void main (void)\
                                                {\
													vec3 bumpRGB = normalize(texture2D(n_texture, outCoord).rgb * 2.0 - 1.0);\
													\
                                                    vec3 N = normalize(normalize(vNormal.xyz) * bumpRGB);\
                                                    vec3 L = normalize(light.pos.xyz - vPosition.xyz);\
                                                    vec3 V = -normalize(vPosition.xyz);\
                                                    vec3 R = -reflect(L, N);\
                                                    vec4 ambient = light.amb * material.amb;\
                                                    float d = length(light.pos.xyz - vPosition.xyz);\
                                                    float denom = light.att.x + light.att.y * d + light.att.z * d * d;\
													\
													vec3 albedo = texture2D(s_texture, outCoord).rgb;\
													vec3 amb = 0.3 * albedo;\
                                                    \
                                                    vec4 diffuse = max(dot(L, N), 0.0) * light.diff * material.diff / denom;\
                                                    \
                                                    vec4 specular = pow(max(dot(R, V), 0.0), material.shine) * light.spec * material.spec / denom;\
                                                    gl_FragColor = texture2D(s_texture, outCoord) + ambient + diffuse + specular;\
                                                }";

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//vec4 diffuse = max(dot(L, N), 0.0) * light.diff * material.diff / denom;\

	glShaderSource(fragmentShader, 1, (const char**)&fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	GLint isShaderCompiled;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isShaderCompiled);

	if (!isShaderCompiled)
	{
		int infoLogLength, charactersWritten;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		std::vector<char> infoLog; infoLog.resize(infoLogLength);
		glGetShaderInfoLog(fragmentShader, infoLogLength, &charactersWritten, infoLog.data());

		MessageBox(nativeWindow, infoLogLength > 1 ? infoLog.data() : _T("Failed to compile fragment shader. (No information)"),
			ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);

		return false;
	}

	// Vertex shader code
	const char* const vertexShaderSource = "\
											precision mediump float;\
											attribute highp vec4	myVertex;\
											attribute highp vec2	inCoord;\
											attribute highp vec4	aNormal;\
											uniform mediump mat4	transformationMatrix;\
											uniform mediump mat4	view;\
											uniform mediump mat4	proj;\
											varying highp vec2 outCoord;\
											varying highp vec4 vPosition;\
											varying highp vec4 vNormal;\
											\highp mat4 transpose(highp mat4 inMatrix){\
												highp vec4 i0 = inMatrix[0];\
												highp vec4 i1 = inMatrix[1];\
												highp vec4 i2 = inMatrix[2];\
												highp vec4 i3 = inMatrix[3];\
											\
												highp mat4 outMatrix = mat4(\
													vec4(i0.x, i1.x, i2.x, i3.x),\
													vec4(i0.y, i1.y, i2.y, i3.y),\
													vec4(i0.z, i1.z, i2.z, i3.z),\
													vec4(i0.w, i1.w, i2.w, i3.w)\
													);\
												return outMatrix;\
											}\
											\
											highp mat4 inverse(mat4 m) {\
											float\
											a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3],\
											a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3],\
											a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3],\
											a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3],\
											\
											b00 = a00 * a11 - a01 * a10,\
											b01 = a00 * a12 - a02 * a10,\
											b02 = a00 * a13 - a03 * a10,\
											b03 = a01 * a12 - a02 * a11,\
											b04 = a01 * a13 - a03 * a11,\
											b05 = a02 * a13 - a03 * a12,\
											b06 = a20 * a31 - a21 * a30,\
											b07 = a20 * a32 - a22 * a30,\
											b08 = a20 * a33 - a23 * a30,\
											b09 = a21 * a32 - a22 * a31,\
											b10 = a21 * a33 - a23 * a31,\
											b11 = a22 * a33 - a23 * a32,\
											\
											det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;\
											\
											return mat4(\
											a11 * b11 - a12 * b10 + a13 * b09,\
											a02 * b10 - a01 * b11 - a03 * b09,\
											a31 * b05 - a32 * b04 + a33 * b03,\
											a22 * b04 - a21 * b05 - a23 * b03,\
											a12 * b08 - a10 * b11 - a13 * b07,\
											a00 * b11 - a02 * b08 + a03 * b07,\
											a32 * b02 - a30 * b05 - a33 * b01,\
											a20 * b05 - a22 * b02 + a23 * b01,\
											a10 * b10 - a11 * b08 + a13 * b06,\
											a01 * b08 - a00 * b10 - a03 * b06,\
											a30 * b04 - a31 * b02 + a33 * b00,\
											a21 * b02 - a20 * b04 - a23 * b00,\
											a11 * b07 - a10 * b09 - a12 * b06,\
											a00 * b09 - a01 * b07 + a02 * b06,\
											a31 * b01 - a30 * b03 - a32 * b00,\
											a20 * b03 - a21 * b01 + a22 * b00) / det;\
											}\
											\
											vec4 rotatez(vec4 pos, float angle){\
												vec4 npos;\
												float sina = sin(angle);\
												float cosa = cos(angle);\
												\
												npos.x = cosa * pos.x - sina * pos.y;\
												npos.y = sina * pos.x + cosa * pos.y;\
												npos.z = pos.z;\
												npos.w = pos.w;\
												\
												return npos;\
											}\
											float len(vec4 pos1, vec4 pos2){\
												float x = pos2.x - pos1.x;\
												float y = pos2.y - pos1.y;\
												float z = pos2.z - pos1.z;\
												\
												return sqrt(x*x + y*y + z*z);\
											}\
											\
											vec4 Bend(vec4 pos, vec4 origin, float amount){\
												vec4 npos;\
												\
												npos = rotatez(pos, amount * len(pos, origin));\
												\
												return npos;\
											}\
											vec4 Taper(vec4 pos){\
												vec4 npos;\
												float tp = 1.0 - (((pos.z + 1.0) / 1.0)/2.0) ;\
												\
												npos.z = pos.z;\
												npos.w = pos.w;\
												if(pos.z < -1.0){\
													npos.y = pos.y;\
													npos.x = pos.x;\
												} else if(pos.z < 1.0){\
													npos.y = tp * pos.y;\
													npos.x = tp * pos.x;\
												} else {\
													npos.y = 0.1;\
													npos.x = 0.1;\
												}\
												\
												return npos;\
											}\
											vec4 Twist(vec4 pos){\
												\
												float sint = sin(pos.z);\
												float cost = cos(pos.z);\
												vec4 npos;\
												\
												npos.x = pos.x * cost - pos.y * sint;\
												npos.y = pos.x * sint + pos.y * cost;\
												npos.w = pos.w;\
												npos.z = pos.z;\
												\
												return npos;\
											}\
											\
											vec4 Vortex(vec4 pos){\
												float thetaMax = 3.14 / 1.0;\
												float p = -(pow(pos.x, 2.0) + pow(pos.y, 2.0));\
												float alpha;\
												float r;\
												vec4 npos;\
												\
												if(pos.z < -1.0){\
													alpha = 0.0;\
												} else if (pos.z < 1.0){\
													r = ((pos.z + 1.0) / 2.0) * thetaMax; ;\
													alpha = r * exp(p);\
												} else {\
													r = thetaMax; \
													alpha = r * exp(p);\
												}\
												\
												npos.x = pos.x * cos(alpha) - pos.y * sin(alpha);\
												npos.y = pos.x * sin(alpha) + pos.y * cos(alpha);\
												npos.w = pos.w;\
												npos.z = pos.z;\
												\
												return npos;\
											}\
											\
											\
											void main(void)\
											{\
												vec4 bendOrigin = vec4(3, 0, -1, 0);\
												\
												vec4 vpos = proj * view * transformationMatrix * myVertex;\
												vec4 npos = Taper(vpos);\
												mat4 mNormal = transpose(inverse(mat4(proj*view*transformationMatrix)));\
												vec4 vnor = mNormal * aNormal;\
												vec4 nnor = Taper(vnor);\
												\
												vNormal = vnor + nnor;\
												vPosition = vpos + npos;\
												gl_Position = vPosition; \
												outCoord = inCoord;\
												\
											}";
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, (const char**)&vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isShaderCompiled);
	if (!isShaderCompiled)
	{
		int infoLogLength, charactersWritten;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		char* infoLog = new char[infoLogLength];
		glGetShaderInfoLog(vertexShader, infoLogLength, &charactersWritten, infoLog);

		MessageBox(nativeWindow, infoLogLength > 1 ? infoLog : _T("Failed to compile vertex shader. (No information)"),
			ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);

		delete[] infoLog;
		return false;
	}

	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, fragmentShader);
	glAttachShader(shaderProgram, vertexShader);
	glBindAttribLocation(shaderProgram, VertexArray, "myVertex");
	glBindAttribLocation(shaderProgram, textureCoord, "inCoord");
	glBindAttribLocation(shaderProgram, NormalArray, "aNormal");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, cubePixelData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureId[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, normalMapData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glLinkProgram(shaderProgram);

	GLint isLinked;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
	if (!isLinked)
	{
		int infoLogLength, charactersWritten;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

		char* infoLog = new char[infoLogLength];
		glGetProgramInfoLog(shaderProgram, infoLogLength, &charactersWritten, infoLog);

		MessageBox(nativeWindow, infoLogLength > 1 ? infoLog : _T("Failed to link GL program object. (No information)"),
			ERROR_TITLE, MB_OK | MB_ICONEXCLAMATION);

		delete[] infoLog;
		return false;
	}
	glUseProgram(shaderProgram);

	if (!testGLError(nativeWindow, "glUseProgram")) { return false; }
	return true;
}

bool renderScene(GLuint vertexBuffer, GLuint texBuffer, GLuint shaderProgram, EGLDisplay eglDisplay, EGLSurface eglSurface, HWND nativeWindow)
{
	if (HasUserQuit) { return false; }

	glClearColor(0.00f, 0.70f, 0.67f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT);

	int matrixLocation = glGetUniformLocation(shaderProgram, "transformationMatrix");
	int textureLocation = glGetUniformLocation(shaderProgram, "s_texture");
	int matrixView = glGetUniformLocation(shaderProgram, "view");
	int projView = glGetUniformLocation(shaderProgram, "proj");

	static float theta = 1.3f;
	static float LoadTransformationMatrix[] =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	mat4 transformationMatrix;
	mat4 tmpMatrix;
	mat4 viewTransform;
	mat4 projTransform;

	tmpMatrix.setTransformationMatrix(LoadTransformationMatrix);

	transformationMatrix.ajouLoadIdentity();
	//transformationMatrix.ajouMultMatrix(tmpMatrix);
	//transformationMatrix.ajouLoadMatrix(tmpMatrix);
	//transformationMatrix.ajouScale3f(0.6f, 1.7f, 1.5f);
	//transformationMatrix.ajouTranslate3f(-1.0f, 0.0f, 0.0f);
	transformationMatrix.ajouRotateX1f(theta*1.5);
	transformationMatrix.ajouRotateZ1f(theta * 8);
	transformationMatrix.ajouRotateY1f(theta * 3);

	viewTransform.ajouLoadIdentity();
	viewTransform.viewSetLookat(0.0, 0.0, 0.0, 2.0, 2.0, 2.0, 0.0, 1.0, 0.0);
	projTransform.projLookat(-2.5, 2.5, -2.5, 2.5, -2.5, 2.5);

	theta += 0.0003f;

	// Pass the transformationMatrix to the shader using its location
	glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, transformationMatrix.getTransformationMatrix());
	glUniformMatrix4fv(matrixView, 1, GL_FALSE, viewTransform.getTransformationMatrix());
	glUniformMatrix4fv(projView, 1, GL_FALSE, projTransform.getTransformationMatrix());

	Light light_in;

	int loc = glGetUniformLocation(shaderProgram, "light.pos");
	glUniform4fv(loc, 1, light_in.pos);
	loc = glGetUniformLocation(shaderProgram, "light.amb");
	glUniform4fv(loc, 1, light_in.amb);
	loc = glGetUniformLocation(shaderProgram, "light.diff");
	glUniform4fv(loc, 1, light_in.diff);
	loc = glGetUniformLocation(shaderProgram, "light.spec");
	glUniform4fv(loc, 1, light_in.spec);
	loc = glGetUniformLocation(shaderProgram, "light.att");
	glUniform4fv(loc, 1, light_in.att);
	loc = glGetUniformLocation(shaderProgram, "light.color");
	glUniform4fv(loc, 1, light_in.color);

	Material material_in;

	loc = glGetUniformLocation(shaderProgram, "material.amb");
	glUniform4fv(loc, 1, material_in.amb);
	loc = glGetUniformLocation(shaderProgram, "material.diff");
	glUniform4fv(loc, 1, material_in.diff);
	loc = glGetUniformLocation(shaderProgram, "material.spec");
	glUniform4fv(loc, 1, material_in.spec);
	loc = glGetUniformLocation(shaderProgram, "material.shine");
	glUniform1f(loc, material_in.shine);

	loc = glGetUniformLocation(shaderProgram, "s_texture");
	glUniform1i(loc, 0);
	loc = glGetUniformLocation(shaderProgram, "n_texture");
	glUniform1i(loc, 1);



	if (!testGLError(nativeWindow, "glUniformMatrix4fv"))
	{
		return false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glEnableVertexAttribArray(NormalArray);
	glEnableVertexAttribArray(textureCoord);
	glEnableVertexAttribArray(VertexArray);
	glVertexAttribPointer(VertexArray, 3, GL_FLOAT, GL_FALSE, 32, 0);
	glVertexAttribPointer(textureCoord, 2, GL_FLOAT, GL_FALSE, 32, (void*)12);
	glVertexAttribPointer(NormalArray, 3, GL_FLOAT, GL_FALSE, 32, (void*)20);

	if (!testGLError(nativeWindow, "glVertexAttribPointer")) { return false; }
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);


	glDrawArrays(GL_TRIANGLES, 0, 36);
	if (!testGLError(nativeWindow, "glDrawArrays")) { return false; }

	if (!eglSwapBuffers(eglDisplay, eglSurface))
	{
		testEGLError(nativeWindow, "eglSwapBuffers");
		return false;
	}

	MSG eventMessage;
	PeekMessage(&eventMessage, nativeWindow, NULL, NULL, PM_REMOVE);
	TranslateMessage(&eventMessage);
	DispatchMessage(&eventMessage);
	return true;
}

void deInitializeGLState(GLuint fragmentShader, GLuint vertexShader, GLuint shaderProgram, GLuint vertexBuffer, GLuint texBuffer)
{
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteProgram(shaderProgram);

	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &texBuffer);
	glDeleteTextures(1, textureId);
}

void releaseEGLState(EGLDisplay eglDisplay)
{
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglTerminate(eglDisplay);
}

void releaseWindowAndDisplay(HWND nativeWindow, HDC deviceContext)
{
	if (deviceContext) { ReleaseDC(nativeWindow, deviceContext); }
	if (nativeWindow) { DestroyWindow(nativeWindow); }
}

/*!*********************************************************************************************************************
\param			applicationInstance         Application instance created by the Operating System
\param			previousInstance            Alwa?ys NULL
\param			commandLineString           Command line string passed from the Operating System, ignored.
\param			showCommand                 Specifies how the window is to be shown, ignored.
\return	Result code to send to the Operating System
\brief	Main function of the program, executes other functions.
***********************************************************************************************************************/
int WINAPI WinMain(HINSTANCE applicationInstance, HINSTANCE previousInstance, TCHAR* /*commandLineString*/, int /*showCommand*/)
{
	// Windows variables
	HWND				nativeWindow = NULL;
	HDC					deviceContext = NULL;

	// EGL variables
	EGLDisplay			eglDisplay = NULL;
	EGLConfig			eglConfig = NULL;
	EGLSurface			eglSurface = NULL;
	EGLContext			eglContext = NULL;

	GLuint fragmentShader = 0, vertexShader = 0;
	GLuint shaderProgram = 0;

	GLuint	vertexBuffer = 0;
	GLuint	texBuffer = 0;

	if (!createWindowAndDisplay(applicationInstance, nativeWindow, deviceContext)) { goto cleanup; }

	if (!createEGLDisplay(deviceContext, eglDisplay)) { goto cleanup; }

	if (!chooseEGLConfig(eglDisplay, eglConfig)) { goto cleanup; }

	if (!createEGLSurface(nativeWindow, eglDisplay, eglConfig, eglSurface)) { goto cleanup; }

	if (!setupEGLContext(eglDisplay, eglConfig, eglSurface, eglContext, nativeWindow)) { goto cleanup; }

	if (!initializeBuffer(vertexBuffer, texBuffer, nativeWindow)) { goto cleanup; }

	if (!initializeShaders(fragmentShader, vertexShader, shaderProgram, nativeWindow)) { goto cleanup; }

	for (int i = 0; i < 333800; ++i)
	{
		if (!renderScene(vertexBuffer, texBuffer, shaderProgram, eglDisplay, eglSurface, nativeWindow)) { break; }
	}

	deInitializeGLState(fragmentShader, vertexShader, shaderProgram, vertexBuffer, texBuffer);


cleanup:
	releaseEGLState(eglDisplay);
	releaseWindowAndDisplay(nativeWindow, deviceContext);
	return 0;
}

