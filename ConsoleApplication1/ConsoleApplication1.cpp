// opengl_wektory_normalne.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


/*
(c) Janusz Ganczarski
http://www.januszg.hg.pl
JanuszG@enter.net.pl
*/

#include <GL/glut.h>
#include <GL/glext.h>
#ifndef WIN32
#define GLX_GLXEXT_LEGACY
#include <GL/glx.h>
#define wglGetProcAddress glXGetProcAddressARB
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "materials.h"
#include "colors.h"



//#include "VectorMath.cpp"

// Some data types
typedef GLfloat GLTVector2[2];      // Two component floating point vector
typedef GLfloat GLTVector3[3];      // Three component floating point vector
typedef GLfloat GLTVector4[4];      // Four component floating point vector
typedef GLfloat GLTMatrix[16];      // A column major 4x4 matrix of type GLfloat

#pragma region MyRegion
#include <math.h>

									// Adds two vectors together
void gltAddVectors(const GLTVector3 vFirst, const GLTVector3 vSecond, GLTVector3 vResult) {
	vResult[0] = vFirst[0] + vSecond[0];
	vResult[1] = vFirst[1] + vSecond[1];
	vResult[2] = vFirst[2] + vSecond[2];
}

// Subtract one vector from another
void gltSubtractVectors(const GLTVector3 vFirst, const GLTVector3 vSecond, GLTVector3 vResult)
{
	vResult[0] = vFirst[0] - vSecond[0];
	vResult[1] = vFirst[1] - vSecond[1];
	vResult[2] = vFirst[2] - vSecond[2];
}

// Scales a vector by a scalar
void gltScaleVector(GLTVector3 vVector, const GLfloat fScale)
{
	vVector[0] *= fScale; vVector[1] *= fScale; vVector[2] *= fScale;
}

// Gets the length of a vector squared
GLfloat gltGetVectorLengthSqrd(const GLTVector3 vVector)
{
	return (vVector[0] * vVector[0]) + (vVector[1] * vVector[1]) + (vVector[2] * vVector[2]);
}

// Gets the length of a vector
GLfloat gltGetVectorLength(const GLTVector3 vVector)
{
	return (GLfloat)sqrt(gltGetVectorLengthSqrd(vVector));
}

// Scales a vector by it's length - creates a unit vector
void gltNormalizeVector(GLTVector3 vNormal)
{
	GLfloat fLength = 1.0f / gltGetVectorLength(vNormal);
	gltScaleVector(vNormal, fLength);
}

// Copies a vector
void gltCopyVector(const GLTVector3 vSource, GLTVector3 vDest)
{
	memcpy(vDest, vSource, sizeof(GLTVector3));
}

// Get the dot product between two vectors
GLfloat gltVectorDotProduct(const GLTVector3 vU, const GLTVector3 vV)
{
	return vU[0] * vV[0] + vU[1] * vV[1] + vU[2] * vV[2];
}

// Calculate the cross product of two vectors
void gltVectorCrossProduct(const GLTVector3 vU, const GLTVector3 vV, GLTVector3 vResult)
{
	vResult[0] = vU[1] * vV[2] - vV[1] * vU[2];
	vResult[1] = -vU[0] * vV[2] + vV[0] * vU[2];
	vResult[2] = vU[0] * vV[1] - vV[0] * vU[1];
}



// Given three points on a plane in counter clockwise order, calculate the unit normal
void gltGetNormalVector(const GLTVector3 vP1, const GLTVector3 vP2, const GLTVector3 vP3, GLTVector3 vNormal)
{
	GLTVector3 vV1, vV2;

	gltSubtractVectors(vP2, vP1, vV1);
	gltSubtractVectors(vP3, vP1, vV2);

	gltVectorCrossProduct(vV1, vV2, vNormal);
	gltNormalizeVector(vNormal);
}



// Transform a point by a 4x4 matrix
void gltTransformPoint(const GLTVector3 vSrcVector, const GLTMatrix mMatrix, GLTVector3 vOut)
{
	vOut[0] = mMatrix[0] * vSrcVector[0] + mMatrix[4] * vSrcVector[1] + mMatrix[8] * vSrcVector[2] + mMatrix[12];
	vOut[1] = mMatrix[1] * vSrcVector[0] + mMatrix[5] * vSrcVector[1] + mMatrix[9] * vSrcVector[2] + mMatrix[13];
	vOut[2] = mMatrix[2] * vSrcVector[0] + mMatrix[6] * vSrcVector[1] + mMatrix[10] * vSrcVector[2] + mMatrix[14];
}

// Rotates a vector using a 4x4 matrix. Translation column is ignored
void gltRotateVector(const GLTVector3 vSrcVector, const GLTMatrix mMatrix, GLTVector3 vOut)
{
	vOut[0] = mMatrix[0] * vSrcVector[0] + mMatrix[4] * vSrcVector[1] + mMatrix[8] * vSrcVector[2];
	vOut[1] = mMatrix[1] * vSrcVector[0] + mMatrix[5] * vSrcVector[1] + mMatrix[9] * vSrcVector[2];
	vOut[2] = mMatrix[2] * vSrcVector[0] + mMatrix[6] * vSrcVector[1] + mMatrix[10] * vSrcVector[2];
}


// Gets the three coefficients of a plane equation given three points on the plane.
void gltGetPlaneEquation(GLTVector3 vPoint1, GLTVector3 vPoint2, GLTVector3 vPoint3, GLTVector3 vPlane)
{
	// Get normal vector from three points. The normal vector is the first three coefficients
	// to the plane equation...
	gltGetNormalVector(vPoint1, vPoint2, vPoint3, vPlane);

	// Final coefficient found by back substitution
	vPlane[3] = -(vPlane[0] * vPoint3[0] + vPlane[1] * vPoint3[1] + vPlane[2] * vPoint3[2]);
}

// Determine the distance of a point from a plane, given the point and the
// equation of the plane.
GLfloat gltDistanceToPlane(GLTVector3 vPoint, GLTVector4 vPlane)
{
	return vPoint[0] * vPlane[0] + vPoint[1] * vPlane[1] + vPoint[2] * vPlane[2] + vPlane[3];
}
#pragma endregion





PFNGLWINDOWPOS2IPROC glWindowPos2i = NULL;

// wskaźnik dostępności rozszerzenia EXT_rescale_normal

bool rescale_normal = false;

// stałe do obsługi menu podręcznego

enum
{
	BRASS,                // mosiądz
	BRONZE,               // brąz
	POLISHED_BRONZE,      // polerowany brąz
	CHROME,               // chrom
	COPPER,               // miedź
	POLISHED_COPPER,      // polerowana miedź
	GOLD,                 // złoto
	POLISHED_GOLD,        // polerowane złoto
	PEWTER,               // grafit (cyna z ołowiem)
	SILVER,               // srebro
	POLISHED_SILVER,      // polerowane srebro
	EMERALD,              // szmaragd
	JADE,                 // jadeit
	OBSIDIAN,             // obsydian
	PEARL,                // perła
	RUBY,                 // rubin
	TURQUOISE,            // turkus
	BLACK_PLASTIC,        // czarny plastik
	BLACK_RUBBER,         // czarna guma

	KIERUNKOWE,
	OTACZAJĄCE,
	REFLEKTOR,




	NORMALS_SMOOTH,       // jeden wektor normalny na wierzchołek
	NORMALS_FLAT,         // jeden wektor normalny na ścianę
	FULL_WINDOW,          // aspekt obrazu - całe okno
	ASPECT_1_1,           // aspekt obrazu 1:1
	EXIT                  // wyjście
};

// aspekt obrazu

int aspect = FULL_WINDOW;

// usunięcie definicji makr near i far

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

// rozmiary bryły obcinania

const GLdouble left = -1.0;
const GLdouble right = 1.0;
const GLdouble bottom = -1.0;
const GLdouble top = 1.0;
const GLdouble near = 3.0;
const GLdouble far = 7.0;

// kąty obrotu

GLfloat rotatex = 0.0;
GLfloat rotatey = 0.0;

// wskaźnik naciśnięcia lewego przycisku myszki

int button_state = GLUT_UP;

// poło¿enie kursora myszki

int button_x, button_y;

// współczynnik skalowania

GLfloat scale = 1.0;

// właściwości materiału - domyślnie mosiądz
//////////////////////////////////////////////////////////////////////////////////////////////
//Tu ustala się kolor
const GLfloat *ambient = PolishedSilverAmbient;
const GLfloat *diffuse = PolishedSilverDiffuse;
const GLfloat *specular = PolishedSilverSpecular;
GLfloat shininess = PolishedSilverShininess;


///////////////////////////////////////////////////////////////////////////////////////////////


GLfloat light_position[4] =
{
	0.0,0.0,2.0,0.0
};

GLfloat ambient_light[4] =
{
	0.2,0.2,0.2,1.0
};

// kąty obrotu kierunku źródła światła

GLfloat light_rotatex = 0.0;
GLfloat light_rotatey = 0.0;

// kąty obrotu położenia źródła światła

// kierunek reflektora

GLfloat spot_direction[3] =
{
	0.0,0.0,-1.0
};

// kąt odciecia reflektora

GLfloat spot_cutoff = 180.0;

// wykładnik tłumienia kątowego reflektora

GLfloat spot_exponent = 128.0;

// stały współczynnik tłumienia światła

GLfloat constant_attenuation = 1.0;

// liniowy współczynnik tłumienia światła

GLfloat linear_attenuation = 0.0;

// kwadratowy współczynnik tłumienia światła

GLfloat quadratic_attenuation = 0.0;

#define GL_PI 3.1415f
// Wielkoci obrotów
static GLfloat xRot = 0.0f;
static GLfloat yRot = 0.0f;

// wektory normalne



int normals = NORMALS_FLAT;

// współrzędne wierzchołków dwudziestościanu

GLfloat vertex[12 * 3] =
{
	0.000,  0.667,  0.500,   // v0
	0.000,  0.667, -0.500,   // v1
	0.000, -0.667, -0.500,   // v2
	0.000, -0.667,  0.500,   // v3
	0.667,  0.500,  0.000,   // v4
	0.667, -0.500,  0.000,   // v5
	-0.667, -0.500,  0.000,  // v6
	-0.667,  0.500,  0.000,  // v7
	0.500,  0.000,  0.667,   // v8
	-0.500,  0.000,  0.667,  // v9
	-0.500,  0.000, -0.667,  // v10
	0.500,  0.000, -0.667    // v11
};

// opis ścian dwudziestościanu

int triangles[20 * 3] =
{
	2, 10, 11,
	1, 11, 10,
	1, 10,  7,
	1,  4, 11,
	0,  1,  7,
	0,  4,  1,
	0,  9,  8,
	3,  8,  9,
	0,  7,  9,
	0,  8,  4,
	3,  9,  6,
	3,  5,  8,
	2,  3,  6,
	2,  5,  3,
	2,  6, 10,
	2, 11,  5,
	6,  7, 10,
	6,  9,  7,
	4,  5, 11,
	4,  8,  5
};

// obliczanie wektora normalnego dla wybranej ściany

void Normal(GLfloat *n, int i)
{
	GLfloat v1[3], v2[3];

	// obliczenie wektorów na podstawie współrzędnych wierzchołków trójkątów
	v1[0] = vertex[3 * triangles[3 * i + 1] + 0] - vertex[3 * triangles[3 * i + 0] + 0];
	v1[1] = vertex[3 * triangles[3 * i + 1] + 1] - vertex[3 * triangles[3 * i + 0] + 1];
	v1[2] = vertex[3 * triangles[3 * i + 1] + 2] - vertex[3 * triangles[3 * i + 0] + 2];
	v2[0] = vertex[3 * triangles[3 * i + 2] + 0] - vertex[3 * triangles[3 * i + 1] + 0];
	v2[1] = vertex[3 * triangles[3 * i + 2] + 1] - vertex[3 * triangles[3 * i + 1] + 1];
	v2[2] = vertex[3 * triangles[3 * i + 2] + 2] - vertex[3 * triangles[3 * i + 1] + 2];

	// obliczenie waktora normalnego przy pomocy iloczynu wektorowego
	n[0] = v1[1] * v2[2] - v1[2] * v2[1];
	n[1] = v1[2] * v2[0] - v1[0] * v2[2];
	n[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

// normalizacja wektora

void Normalize(GLfloat *v)
{
	// obliczenie długości wektora
	GLfloat d = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	// normalizacja wektora
	if (d)
	{
		v[0] /= d;
		v[1] /= d;
		v[2] /= d;
	}
}

int light = KIERUNKOWE;
// funkcja generująca scenę 3D

void Display()
{
	// kolor tła - zawartość bufora koloru
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// czyszczenie bufora koloru i bufora głębokości
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// wybór macierzy modelowania
	glMatrixMode(GL_MODELVIEW);

	// macierz modelowania = macierz jednostkowa
	glLoadIdentity();

	// przesunięcie układu współrzędnych obiektu do środka bryły odcinania
	glTranslatef(0, 0, -(near + far) / 2);

	// obroty obiektu
	glRotatef(rotatex, 1.0, 0, 0);
	glRotatef(rotatey, 0, 1.0, 0);

	// skalowanie obiektu - klawisze "+" i "-"
	glScalef(scale, scale, scale);

	// włączenie testu bufora głębokości
	glEnable(GL_DEPTH_TEST);



	// właściwości materiału
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);


	switch (light)
	{
	case KIERUNKOWE:
		// włączenie oświetlenia
		glEnable(GL_LIGHTING);

		// włączenie światła GL_LIGHT0 z parametrami domyślnymi
		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);


		// ustalenie kierunku źródła światła
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
		break;
	case OTACZAJĄCE:
		// włączenie oświetlenia
		glEnable(GL_LIGHTING);

		// włączenie światła GL_LIGHT0 z parametrami domyślnymi
		glEnable(GL_LIGHT0);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_light);
		glDisable(GL_LIGHTING);
		GLfloat rgba[4];
		glColor3fv(Black);
		glGetFloatv(GL_LIGHT_MODEL_AMBIENT, rgba);
		break;
	case REFLEKTOR:
		// włączenie oświetlenia
		glEnable(GL_LIGHTING);

		// włączenie światła GL_LIGHT0
		glEnable(GL_LIGHT0);
		// kąt odciecia reflektora
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, spot_cutoff);

		// wykładnik tłumienia kątowego reflektora
		glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, spot_exponent);

		// stały współczynnik tłumienia światła,
		glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, constant_attenuation);

		// liniowy współczynnik tłumienia światła
		glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, linear_attenuation);

		// kwadratowy współczynnik tłumienia światła
		glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, quadratic_attenuation);

		// zmiana położenia źródła światła jest wykonywana niezależnie
		// od obrotów obiektu, stąd odłożenie na stos macierzy modelowania
		glPushMatrix();


		// macierz modelowania = macierz jednostkowa
		glLoadIdentity();

		// przesuniecie układu współrzednych źródła światła do środka bryły odcinania
		glTranslatef(0, 0, -(near + far) / 2);

		// obroty położenia źródła światła - klawisze kursora
		glRotatef(light_rotatex, 1.0, 0, 0);
		glRotatef(light_rotatey, 0, 1.0, 0);

		// przesuniecie źródła światła
		glTranslatef(light_position[0], light_position[1], light_position[2]);

		// ustalenie pozycji źródła światła
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);

		// ustalenie kierunku reflektora
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction);

		// odłożenie na stos zmiennych stanu związanych z oświetleniem sceny
		glPushAttrib(GL_LIGHTING_BIT);
		break;


	default:
		break;
	}







	// włączenie automatycznej normalizacji wektorów normalnych
	// lub automatycznego skalowania jednostkowych wektorów normalnych
	if (rescale_normal == true)
		glEnable(GL_RESCALE_NORMAL);
	else
		glEnable(GL_NORMALIZE);

	// początek definicji obiektu
	glBegin(GL_TRIANGLES);

	// generowanie obiektu gładkiego - jeden uśredniony
	// wektor normalny na wierzchołek
	if (normals == NORMALS_SMOOTH)
		//for (int i = 0; i < 20; i++)
		//{
		//	// obliczanie wektora normalnego dla pierwszego wierzchołka
		//	GLfloat n[3];
		//	n[0] = n[1] = n[2] = 0.0;
		//	// wyszukanie wszystkich ścian posiadających bie¿ący wierzchołek
		//	for (int j = 0; j < 20; j++)
		//		if (3 * triangles[3 * i + 0] == 3 * triangles[3 * j + 0] ||
		//			3 * triangles[3 * i + 0] == 3 * triangles[3 * j + 1] ||
		//			3 * triangles[3 * i + 0] == 3 * triangles[3 * j + 2])
		//		{
		//			// dodawanie wektorów normalnych poszczególnych ścian
		//			GLfloat nv[3];
		//			Normal(nv, j);
		//			n[0] += nv[0];
		//			n[1] += nv[1];
		//			n[2] += nv[2];
		//		}
		//	// uśredniony wektor normalny jest normalizowany tylko, gdy biblioteka
		//	// obsługuje automatyczne skalowania jednostkowych wektorów normalnych
		//	if (rescale_normal == true)
		//		Normalize(n);
		//	glNormal3fv(n);
		//	glVertex3fv(&vertex[3 * triangles[3 * i + 0]]);
		//	// obliczanie wektora normalnego dla drugiego wierzchołka
		//	n[0] = n[1] = n[2] = 0.0;
		//	// wyszukanie wszystkich ścian posiadających bie¿ący wierzchołek
		//	for (int j = 0; j < 20; j++)
		//		if (3 * triangles[3 * i + 1] == 3 * triangles[3 * j + 0] ||
		//			3 * triangles[3 * i + 1] == 3 * triangles[3 * j + 1] ||
		//			3 * triangles[3 * i + 1] == 3 * triangles[3 * j + 2])
		//		{
		//			// dodawanie wektorów normalnych poszczególnych ścian
		//			GLfloat nv[3];
		//			Normal(nv, j);
		//			n[0] += nv[0];
		//			n[1] += nv[1];
		//			n[2] += nv[2];
		//		}
		//	// uśredniony wektor normalny jest normalizowany tylko, gdy biblioteka
		//	// obsługuje automatyczne skalowania jednostkowych wektorów normalnych
		//	if (rescale_normal == true)
		//		Normalize(n);
		//	glNormal3fv(n);
		//	glVertex3fv(&vertex[3 * triangles[3 * i + 1]]);
		//	// obliczanie wektora normalnego dla trzeciego wierzchołka
		//	n[0] = n[1] = n[2] = 0.0;
		//	// wyszukanie wszystkich ścian posiadających bie¿ący wierzchołek
		//	for (int j = 0; j < 20; j++)
		//		if (3 * triangles[3 * i + 2] == 3 * triangles[3 * j + 0] ||
		//			3 * triangles[3 * i + 2] == 3 * triangles[3 * j + 1] ||
		//			3 * triangles[3 * i + 2] == 3 * triangles[3 * j + 2])
		//		{
		//			// dodawanie wektorów normalnych poszczególnych ścian
		//			GLfloat nv[3];
		//			Normal(nv, j);
		//			n[0] += nv[0];
		//			n[1] += nv[1];
		//			n[2] += nv[2];
		//		}
		//	// uśredniony wektor normalny jest normalizowany tylko, gdy biblioteka
		//	// obsługuje automatyczne skalowania jednostkowych wektorów normalnych
		//	if (rescale_normal == true)
		//		Normalize(n);
		//	glNormal3fv(n);
		//	glVertex3fv(&vertex[3 * triangles[3 * i + 2]]);
		//}

	{
		///////////////////////////////////////////////////////////////////////////////////////////////
		//tu się ustala podstawę ostrosłupa
		GLint n = 9;
		GLTVector3 vNormal;
		GLTVector3 vCorners[12] =
		{
			{ 0.0f, 1.0f, 0.0f }, // Góra 0
		};

		for (GLint i = 1; i <= n; i++)
		{

			vCorners[i][0] = vCorners[i - 1][0] + ((0.5*cos(i * 2 * GL_PI / n)) * 1.0f);
			vCorners[i][2] = vCorners[i - 1][2] + ((0.5*sin(i * 2 * GL_PI / n)) * 1.0f);

		}


		//wyznaczanie środka
		GLfloat sx = 0, sz = 0;

		for (GLint i = 1; i <= n; i++)
		{
			sx = sx + vCorners[i][0];
			sz = sz + vCorners[i][2];
		}

		vCorners[0][0] = sx / n;
		vCorners[0][1] = 1.f;
		vCorners[0][2] = sz / n;

		vCorners[11][0] = sx / n;
		vCorners[11][1] = 0.f;
		vCorners[11][2] = sz / n;

		//xRot = vCorners[11][0];

		// Czyszczenie okna aktualnym kolorem czyszczącym
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Zapisanie stanu macierzy i wykonanie obrotów
		glPushMatrix();
		// Cofnięcie obiektów
		glTranslatef(0.0f, -0.25f, -4.0f);
		glRotatef(xRot, 1.0f, 0.0f, 0.0f);
		glRotatef(yRot, 0.0f, 1.0f, 0.0f);
		// Rysowanie piramidy
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_TRIANGLES);

		glNormal3f(0.0f, -1.0f, 0.0f);

		for (GLint i = 1; i < n; i++)
		{
			glTexCoord2f(1.0f, 1.0f);
			glVertex3fv(vCorners[i]);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3fv(vCorners[i + 1]);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3fv(vCorners[11]);
		}

		glTexCoord2f(1.0f, 1.0f);
		glVertex3fv(vCorners[n]);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3fv(vCorners[1]);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3fv(vCorners[11]);


		for (GLint i = 1; i < n; i++)
		{
			gltGetNormalVector(vCorners[i], vCorners[i + 1], vCorners[0], vNormal);
			glNormal3fv(vNormal);
			glTexCoord2f(0.5f, 1.0f);
			glVertex3fv(vCorners[0]);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3fv(vCorners[i + 1]);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3fv(vCorners[i]);
		}

		gltGetNormalVector(vCorners[n], vCorners[1], vCorners[0], vNormal);
		glNormal3fv(vNormal);
		glTexCoord2f(0.5f, 1.0f);
		glVertex3fv(vCorners[0]);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3fv(vCorners[1]);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3fv(vCorners[n]);
	}
	else
	{
		//// generowanie obiektu "płaskiego" - jeden wektor normalny na ścianę
		//for (int i = 0; i < 20; i++)
		//{
		//	GLfloat n[3];
		//	Normal(n, i);

		//	// uśredniony wektor normalny jest normalizowany tylko, gdy biblioteka
		//	// obsługuje automatyczne skalowania jednostkowych wektorów normalnych
		//	if (rescale_normal == true)
		//		Normalize(n);
		//	glNormal3fv(n);
		//	glVertex3fv(&vertex[3 * triangles[3 * i + 0]]);
		//	glVertex3fv(&vertex[3 * triangles[3 * i + 1]]);
		//	glVertex3fv(&vertex[3 * triangles[3 * i + 2]]);
		//}
		GLint n = 9;
		GLTVector3 vNormal;
		GLTVector3 vCorners[12] =
		{
			{ 0.0f, 1.0f, 0.0f }, // Góra 0
		};

		for (GLint i = 1; i <= n; i++)
		{

			vCorners[i][0] = vCorners[i - 1][0] + ((0.5*cos(i * 2 * GL_PI / n)) * 1.0f);
			vCorners[i][2] = vCorners[i - 1][2] + ((0.5*sin(i * 2 * GL_PI / n)) * 1.0f);

		}


		//wyznaczanie środka
		GLfloat sx = 0, sz = 0;

		for (GLint i = 1; i <= n; i++)
		{
			sx = sx + vCorners[i][0];
			sz = sz + vCorners[i][2];
		}

		vCorners[0][0] = sx / n;
		vCorners[0][1] = 1.f;
		vCorners[0][2] = sz / n;

		vCorners[11][0] = sx / n;
		vCorners[11][1] = 0.f;
		vCorners[11][2] = sz / n;

		//xRot = vCorners[11][0];

		// Czyszczenie okna aktualnym kolorem czyszczącym
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Zapisanie stanu macierzy i wykonanie obrotów
		glPushMatrix();
		// Cofnięcie obiektów
		glTranslatef(0.0f, -0.25f, -4.0f);
		glRotatef(xRot, 1.0f, 0.0f, 0.0f);
		glRotatef(yRot, 0.0f, 1.0f, 0.0f);
		// Rysowanie piramidy
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_TRIANGLES);

		glNormal3f(0.0f, -1.0f, 0.0f);

		for (GLint i = 1; i < n; i++)
		{
			glTexCoord2f(1.0f, 1.0f);
			glVertex3fv(vCorners[i]);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3fv(vCorners[i + 1]);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3fv(vCorners[11]);
		}

		glTexCoord2f(1.0f, 1.0f);
		glVertex3fv(vCorners[n]);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3fv(vCorners[1]);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3fv(vCorners[11]);


		for (GLint i = 1; i < n; i++)
		{
			gltGetNormalVector(vCorners[i], vCorners[i + 1], vCorners[0], vNormal);
			glNormal3fv(vNormal);
			glTexCoord2f(0.5f, 1.0f);
			glVertex3fv(vCorners[0]);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3fv(vCorners[i + 1]);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3fv(vCorners[i]);
		}

		gltGetNormalVector(vCorners[n], vCorners[1], vCorners[0], vNormal);
		glNormal3fv(vNormal);
		glTexCoord2f(0.5f, 1.0f);
		glVertex3fv(vCorners[0]);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3fv(vCorners[1]);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3fv(vCorners[n]);
	}


	// koniec definicji obiektu
	glEnd();

	// informacje o modyfikowanych wartościach
	// parametrów źródła światała GL_LIGHT0
	char string[200];
	GLfloat vec[4];
	glColor3fv(Black);

	// kierunek źródła światła
	glGetLightfv(GL_LIGHT0, GL_POSITION, vec);
	sprintf(string, "GL_POSITION = (%f,%f,%f,%f)", vec[0], vec[1], vec[2], vec[3]);


	// kąty obrotu kierunku źródła światła
	sprintf(string, "light_rotatex = %f", light_rotatex);

	sprintf(string, "light_rotatey = %f", light_rotatey);
	;
	// skierowanie poleceñ do wykonania
	glFlush();

	// zamiana buforów koloru
	glutSwapBuffers();
}

// zmiana wielkości okna

void Reshape(int width, int height)
{
	// obszar renderingu - całe okno
	glViewport(0, 0, width, height);

	// wybór macierzy rzutowania
	glMatrixMode(GL_PROJECTION);

	// macierz rzutowania = macierz jednostkowa
	glLoadIdentity();

	// parametry bryły obcinania
	if (aspect == ASPECT_1_1)
	{
		// wysokość okna większa od wysokości okna
		if (width < height && width > 0)
			glFrustum(left, right, bottom*height / width, top*height / width, near, far);
		else

			// szerokość okna większa lub równa wysokości okna
			if (width >= height && height > 0)
				glFrustum(left*width / height, right*width / height, bottom, top, near, far);
	}
	else
		glFrustum(left, right, bottom, top, near, far);

	// generowanie sceny 3D
	Display();
}

// obsługa klawiatury

void Keyboard(unsigned char key, int x, int y)
{
	// klawisz +
	if (key == '+')
		scale += 0.05;
	else

		// klawisz -
		if (key == '-' && scale > 0.05)
			scale -= 0.05;

	// narysowanie sceny
	Display();
}

// obsługa klawiszy funkcyjnych i klawiszy kursora

void SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
		// kursor w lewo
	case GLUT_KEY_LEFT:
		rotatey -= 1;
		break;

		// kursor w górę
	case GLUT_KEY_UP:
		rotatex -= 1;
		break;

		// kursor w prawo
	case GLUT_KEY_RIGHT:
		rotatey += 1;
		break;

		// kursor w dół
	case GLUT_KEY_DOWN:
		rotatex += 1;
		break;
	}

	// odrysowanie okna
	Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

// obsługa przycisków myszki

void MouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		// zapamiętanie stanu lewego przycisku myszki
		button_state = state;

		// zapamiętanie poło¿enia kursora myszki
		if (state == GLUT_DOWN)
		{
			button_x = x;
			button_y = y;
		}
	}
}

// obsługa ruchu kursora myszki

void MouseMotion(int x, int y)
{
	if (button_state == GLUT_DOWN)
	{
		rotatey += 30 * (right - left) / glutGet(GLUT_WINDOW_WIDTH) * (x - button_x);
		button_x = x;
		rotatex -= 30 * (top - bottom) / glutGet(GLUT_WINDOW_HEIGHT) * (button_y - y);
		button_y = y;
		glutPostRedisplay();
	}
}

// obsługa menu podręcznego

void Menu(int value)
{
	switch (value)
	{
		// materiał - mosiądz
	case BRASS:
		ambient = BrassAmbient;
		diffuse = BrassDiffuse;
		specular = BrassSpecular;
		shininess = BrassShininess;
		Display();
		break;

		// materiał - brąz
	case BRONZE:
		ambient = BronzeAmbient;
		diffuse = BronzeDiffuse;
		specular = BronzeSpecular;
		shininess = BronzeShininess;
		Display();
		break;

		// materiał - polerowany brąz
	case POLISHED_BRONZE:
		ambient = PolishedBronzeAmbient;
		diffuse = PolishedBronzeDiffuse;
		specular = PolishedBronzeSpecular;
		shininess = PolishedBronzeShininess;
		Display();
		break;

		// materiał - chrom
	case CHROME:
		ambient = ChromeAmbient;
		diffuse = ChromeDiffuse;
		specular = ChromeSpecular;
		shininess = ChromeShininess;
		Display();
		break;

		// materiał - miedź
	case COPPER:
		ambient = CopperAmbient;
		diffuse = CopperDiffuse;
		specular = CopperSpecular;
		shininess = CopperShininess;
		Display();
		break;

		// materiał - polerowana miedź
	case POLISHED_COPPER:
		ambient = PolishedCopperAmbient;
		diffuse = PolishedCopperDiffuse;
		specular = PolishedCopperSpecular;
		shininess = PolishedCopperShininess;
		Display();
		break;

		// materiał - złoto
	case GOLD:
		ambient = GoldAmbient;
		diffuse = GoldDiffuse;
		specular = GoldSpecular;
		shininess = GoldShininess;
		Display();
		break;

		// materiał - polerowane złoto
	case POLISHED_GOLD:
		ambient = PolishedGoldAmbient;
		diffuse = PolishedGoldDiffuse;
		specular = PolishedGoldSpecular;
		shininess = PolishedGoldShininess;
		Display();
		break;

		// materiał - grafit (cyna z ołowiem)
	case PEWTER:
		ambient = PewterAmbient;
		diffuse = PewterDiffuse;
		specular = PewterSpecular;
		shininess = PewterShininess;
		Display();
		break;

		// materiał - srebro
	case SILVER:
		ambient = SilverAmbient;
		diffuse = SilverDiffuse;
		specular = SilverSpecular;
		shininess = SilverShininess;
		Display();
		break;

		// materiał - polerowane srebro
	case POLISHED_SILVER:
		ambient = PolishedSilverAmbient;
		diffuse = PolishedSilverDiffuse;
		specular = PolishedSilverSpecular;
		shininess = PolishedSilverShininess;
		Display();
		break;

		// materiał - szmaragd
	case EMERALD:
		ambient = EmeraldAmbient;
		diffuse = EmeraldDiffuse;
		specular = EmeraldSpecular;
		shininess = EmeraldShininess;
		Display();
		break;

		// materiał - jadeit
	case JADE:
		ambient = JadeAmbient;
		diffuse = JadeDiffuse;
		specular = JadeSpecular;
		shininess = JadeShininess;
		Display();
		break;

		// materiał - obsydian
	case OBSIDIAN:
		ambient = ObsidianAmbient;
		diffuse = ObsidianDiffuse;
		specular = ObsidianSpecular;
		shininess = ObsidianShininess;
		Display();
		break;

		// materiał - perła
	case PEARL:
		ambient = PearlAmbient;
		diffuse = PearlDiffuse;
		specular = PearlSpecular;
		shininess = PearlShininess;
		Display();
		break;

		// metariał - rubin
	case RUBY:
		ambient = RubyAmbient;
		diffuse = RubyDiffuse;
		specular = RubySpecular;
		shininess = RubyShininess;
		Display();
		break;

		// materiał - turkus
	case TURQUOISE:
		ambient = TurquoiseAmbient;
		diffuse = TurquoiseDiffuse;
		specular = TurquoiseSpecular;
		shininess = TurquoiseShininess;
		Display();
		break;

		// materiał - czarny plastik
	case BLACK_PLASTIC:
		ambient = BlackPlasticAmbient;
		diffuse = BlackPlasticDiffuse;
		specular = BlackPlasticSpecular;
		shininess = BlackPlasticShininess;
		Display();
		break;

		// materiał - czarna guma
	case BLACK_RUBBER:
		ambient = BlackRubberAmbient;
		diffuse = BlackRubberDiffuse;
		specular = BlackRubberSpecular;
		shininess = BlackRubberShininess;
		Display();
		break;

	case REFLEKTOR:
		light = REFLEKTOR;
		Display();
		break;
	case KIERUNKOWE:
		light = KIERUNKOWE;
		Display();
		break;
	case OTACZAJĄCE:
		light = OTACZAJĄCE;
		Display();
		break;


		// wektory normalne - GLU_SMOOTH
	case NORMALS_SMOOTH:
		normals = NORMALS_SMOOTH;
		Display();
		break;

		// wektory normalne - GLU_FLAT
	case NORMALS_FLAT:
		normals = NORMALS_FLAT;
		Display();
		break;

		// obszar renderingu - całe okno
	case FULL_WINDOW:
		aspect = FULL_WINDOW;
		Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;

		// obszar renderingu - aspekt 1:1
	case ASPECT_1_1:
		aspect = ASPECT_1_1;
		Reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
		break;

		// wyjście
	case EXIT:
		exit(0);
	}
}

// sprawdzenie i przygotowanie obsługi wybranych rozszerzeñ

void ExtensionSetup()
{
	// pobranie numeru wersji biblioteki OpenGL
	const char *version = (char*)glGetString(GL_VERSION);

	// odczyt wersji OpenGL
	int major = 0, minor = 0;
	if (sscanf(version, "%d.%d", &major, &minor) != 2)
	{
#ifndef WIN32
		printf("Błędny format wersji OpenGL\n");
#else

		printf("Bledny format wersji OpenGL\n");
#endif

		exit(0);
	}

	// sprawdzenie czy jest co najmniej wersja 1.2
	if (major > 1 || minor >= 2)
		rescale_normal = true;
	else
		// sprawdzenie czy jest obsługiwane rozszerzenie EXT_rescale_normal
		if (glutExtensionSupported("GL_EXT_rescale_normal"))
			rescale_normal = true;
}

int main(int argc, char *argv[])
{
	// inicjalizacja biblioteki GLUT
	glutInit(&argc, argv);

	// inicjalizacja bufora ramki
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// rozmiary głównego okna programu
	glutInitWindowSize(500, 500);

	// utworzenie głównego okna programu
	glutCreateWindow("Wektory normalne");

	// dołączenie funkcji generującej scenę 3D
	glutDisplayFunc(Display);

	// dołączenie funkcji wywoływanej przy zmianie rozmiaru okna
	glutReshapeFunc(Reshape);

	// dołączenie funkcji obsługi klawiatury
	glutKeyboardFunc(Keyboard);

	// dołączenie funkcji obsługi klawiszy funkcyjnych i klawiszy kursora
	glutSpecialFunc(SpecialKeys);

	// obsługa przycisków myszki
	glutMouseFunc(MouseButton);

	// obsługa ruchu kursora myszki
	glutMotionFunc(MouseMotion);

	// utworzenie menu podręcznego
	glutCreateMenu(Menu);

	// utworzenie podmenu - Materiał
	int MenuMaterial = glutCreateMenu(Menu);
#ifdef WIN32

	glutAddMenuEntry("Mosiądz", BRASS);
	glutAddMenuEntry("Brąz", BRONZE);
	glutAddMenuEntry("Polerowany brąz", POLISHED_BRONZE);
	glutAddMenuEntry("Chrom", CHROME);
	glutAddMenuEntry("Miedź", COPPER);
	glutAddMenuEntry("Polerowana miedź", POLISHED_COPPER);
	glutAddMenuEntry("Złoto", GOLD);
	glutAddMenuEntry("Polerowane złoto", POLISHED_GOLD);
	glutAddMenuEntry("Grafit (cyna z ołowiem)", PEWTER);
	glutAddMenuEntry("Srebro", SILVER);
	glutAddMenuEntry("Polerowane srebro", POLISHED_SILVER);
	glutAddMenuEntry("Szmaragd", EMERALD);
	glutAddMenuEntry("Jadeit", JADE);
	glutAddMenuEntry("Obsydian", OBSIDIAN);
	glutAddMenuEntry("Perła", PEARL);
	glutAddMenuEntry("Rubin", RUBY);
	glutAddMenuEntry("Turkus", TURQUOISE);
	glutAddMenuEntry("Czarny plastik", BLACK_PLASTIC);
	glutAddMenuEntry("Czarna guma", BLACK_RUBBER);
#else

	glutAddMenuEntry("Mosiadz", BRASS);
	glutAddMenuEntry("Braz", BRONZE);
	glutAddMenuEntry("Polerowany braz", POLISHED_BRONZE);
	glutAddMenuEntry("Chrom", CHROME);
	glutAddMenuEntry("Miedz", COPPER);
	glutAddMenuEntry("Polerowana miedz", POLISHED_COPPER);
	glutAddMenuEntry("Zloto", GOLD);
	glutAddMenuEntry("Polerowane zloto", POLISHED_GOLD);
	glutAddMenuEntry("Grafit (cyna z ołowiem)", PEWTER);
	glutAddMenuEntry("Srebro", SILVER);
	glutAddMenuEntry("Polerowane srebro", POLISHED_SILVER);
	glutAddMenuEntry("Szmaragd", EMERALD);
	glutAddMenuEntry("Jadeit", JADE);
	glutAddMenuEntry("Obsydian", OBSIDIAN);
	glutAddMenuEntry("Perla", PEARL);
	glutAddMenuEntry("Rubin", RUBY);
	glutAddMenuEntry("Turkus", TURQUOISE);
	glutAddMenuEntry("Czarny plastik", BLACK_PLASTIC);
	glutAddMenuEntry("Czarna guma", BLACK_RUBBER);
#endif

	// utworzenie podmenu - Wektory normalne
	int MenuNormals = glutCreateMenu(Menu);
#ifndef WIN32

	glutAddMenuEntry("Jeden wektor normalny na wierzcholek", NORMALS_SMOOTH);
	glutAddMenuEntry("Jeden wektor normalny na sciane", NORMALS_FLAT);
#else

	glutAddMenuEntry("Jeden wektor normalny na wierzchołek", NORMALS_SMOOTH);
	glutAddMenuEntry("Jeden wektor normalny na ścianę", NORMALS_FLAT);
#endif

	// utworzenie podmenu - aspekt obrazu
	int MenuAspect = glutCreateMenu(Menu);
#ifndef WIN32

	glutAddMenuEntry("Aspekt obrazu - całe okno", FULL_WINDOW);
#else

	glutAddMenuEntry("Aspekt obrazu - cale okno", FULL_WINDOW);
#endif

	glutAddMenuEntry("Aspekt obrazu 1:1", ASPECT_1_1);

	int MenuLight = glutCreateMenu(Menu);

	glutAddMenuEntry("Kierunkowe", KIERUNKOWE);
	glutAddMenuEntry("Otaczające", OTACZAJĄCE);
	glutAddMenuEntry("Reflektor", REFLEKTOR);


	// menu główne
	glutCreateMenu(Menu);

#ifdef WIN32

	glutAddSubMenu("Swiatlo", MenuLight);
#else

	glutAddSubMenu("Swiatlo", MenuLight);
#endif

#ifdef WIN32

	glutAddSubMenu("Materiał", MenuMaterial);
#else

	glutAddSubMenu("Material", MenuMaterial);
#endif

	glutAddSubMenu("Wektory normalne", MenuNormals);
	glutAddSubMenu("Aspekt obrazu", MenuAspect);
#ifndef WIN32

	glutAddMenuEntry("Wyjście", EXIT);
#else

	glutAddMenuEntry("Wyjscie", EXIT);
#endif

	// określenie przycisku myszki obsługującej menu podręczne
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// sprawdzenie i przygotowanie obsługi wybranych rozszerzeñ
	ExtensionSetup();

	// wprowadzenie programu do obsługi pętli komunikatów
	glutMainLoop();
	return 0;
}