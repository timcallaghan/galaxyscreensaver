// A class for defining galaxies

#ifndef GALAXY_H
#define GALAXY_H

#include "DX/include/stdafx.h"
#include "DX/include/CVertexBuffer.h"
#include "DX/include/CIndexBuffer.h"
#include "DX/include/CWorldTransform.h"

class Galaxy
{
public:
	Galaxy();
	~Galaxy();

	enum GalaxyType
	{
		ELLIPTICAL,
		SPIRAL
	};

	// A custom vertex structure that store positions and a colour value
	// for reprenting points in 3D space.
	struct CUSTOMVERTEX
	{
		float x, y, z; // Position in 3d space
		DWORD color;   // Color 
	};

	// Galaxy init and render functions
	void GetRegistryConfig(HWND hDlg = NULL); // Gets the galaxy params from the registry
	void SetRegistryConfig(HWND hDlg = NULL); // Sets the galaxy params in the registry
	void GetDialogValues(HWND hDlg);
	void SetDialogValues(HWND hDlg);
	void Initialize(); // Function to initialise the galaxy
	void UpdateGalaxy(); // Function to update all the stars positions in the current animation
	void RenderGalaxy(); // Function to render the current galaxy representation
	float NormalGuassianNumber(); // Function to produce normally distributed numbers with mean zero and variance 1.
	void ResetDefaults(HWND hDlg = NULL);

	// Initialisation and reset functions for windows and D3D
	void SetupWndandTimer(HWND& hWnd, UINT_PTR timer = NULL);
	BOOL InitDirectX();
	BOOL BuildPresentationParameters();
	void ResetDevice();
	void Release(); // Release all resources

	// Member variables

	// General parameters
	//const static int m_nNumberOfStars = 40000;
	int m_nNumberOfStars;
	float m_RadialSpread;
	float m_RotationRate;
	GalaxyType m_GalaxyType;
	float m_SizeOfStars;

	// Elliptical galaxy parameters
	float m_VerticalSpread;
	float m_VerticalExpFactor;

	// Spiral Galaaxy parameters
	float m_SpiralTurnsFactor; // Controls how quickly the spiral "expands"
	float m_RadialScaling; // Expands or shrinks the normal distribution generated radius
	float m_RadialSpreadScaling; // Controls deflection of spiral arms from original radius.

	// Vector that holds the following:
	// 1 - The radius of all stars from the axis of rotation (y axis)
	// 2 - The height above the galactic plane (y value)
	// 3 - The angle of rotation in radians from the starting x-axis aligned position
	// 4 - The brightness of the star (0 to 255)
	D3DXVECTOR4* m_RadiusYThetaBrightness;
	
	// Direct3D and windows related members
	int m_nWidth;
	int m_nHeight; 
	UINT_PTR m_Timer;
	LPDIRECT3D9 m_pD3D9;
	LPDIRECT3DDEVICE9 m_pD3DDevice;
	D3DDISPLAYMODE m_displayMode;
	D3DPRESENT_PARAMETERS m_D3Dpp;
	HWND m_hWnd;
	CWorldTransform m_transform; 	// World transformation matrix for the galaxy
	CVertexBuffer m_VB;		// A dynamic vertex buffer for all of the stars in the galaxy
	bool m_fD3DInitialised;

	float m_StartingXRotation;
	float m_RelativeRotationAngle;

	BOOL m_fFixedCameraPostion;
	float m_ViewingAngle;
	float m_FixedViewingDistance;
};

#endif // GALAXY_H