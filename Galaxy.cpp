#include "Galaxy.h"
#include "resource.h"
#include <sstream>

Galaxy::Galaxy()
{
	// Setup defaults
	m_Timer = NULL;

	m_nWidth = 1;
	m_nHeight = 1;
	m_pD3D9 = NULL;
	m_pD3DDevice = NULL;
	m_fD3DInitialised = false;
	m_RadiusYThetaBrightness = NULL;

	ResetDefaults();	
}

Galaxy::~Galaxy()
{
	Release();
}

void Galaxy::ResetDefaults(HWND hDlg)
{
	// General parameters
	m_GalaxyType = SPIRAL;
	m_nNumberOfStars = 100000;
	m_RadialSpread = 3.0f;
	m_RotationRate = 0.5f;
	m_SizeOfStars = 1.0f;

	// Elliptical galaxy parameters
	m_VerticalSpread = 0.25f;
	m_VerticalExpFactor = 0.5f;

	// Spiral Galaaxy parameters
	m_SpiralTurnsFactor = 0.15f; // Controls how quickly the spiral "expands"
	m_RadialScaling = 0.4f; // Expands or shrinks the normal distribution generated radius
	m_RadialSpreadScaling = 0.05f; // Controls deflection of spiral arms from original radius.

	m_RelativeRotationAngle = 0.0f;

	m_fFixedCameraPostion = TRUE;
	m_ViewingAngle = 45.0f;

	m_FixedViewingDistance = 4.0f;

	if (hDlg != NULL)
	{
		SetDialogValues(hDlg);
	}
}

float Galaxy::NormalGuassianNumber()
{
	static int iset = 0;
	static float gset;
	float fac, rsq, v1, v2;

	if (iset == 0)
	{
		do
		{
			v1 = 2.0f*(((float)rand() + 1.0f)/RAND_MAX) - 1.0f;
			v2 = 2.0f*(((float)rand() + 1.0f)/RAND_MAX) - 1.0f;
			rsq = v1*v1 + v2*v2;
		}
		while (rsq >= 1.0f || rsq == 0.0f);

		fac = sqrt(-2.0f*log(rsq)/rsq);

		// Now make the Box-Muller transformation to get two normal deviates.
		// Return one and save the other for the next time the function is called.
		gset = v1*fac;
		iset = 1;
		return v2*fac;
	}
	else
	{
		iset = 0;
		return m_RadialSpread*gset;
	}
}

void Galaxy::Initialize()
{
	if (m_RadiusYThetaBrightness != NULL)
	{
		delete[] m_RadiusYThetaBrightness;
		m_RadiusYThetaBrightness = NULL;
	}

    m_RadiusYThetaBrightness = new D3DXVECTOR4[m_nNumberOfStars];

    // Seed random number generator
    srand( (UINT)time(NULL) );

	float fRadius;
	float fTheta;
	float fRadialSpread;
	for (int i = 0; i < m_nNumberOfStars; ++i)
	{
		switch (m_GalaxyType)
		{
			case ELLIPTICAL:
			{
				// This is for elliptical galaxies
				// Work out our normally distributed radius and height plus the starting angle.
				fRadius = NormalGuassianNumber();
				fTheta = 2.0f*D3DX_PI*(((float)rand() + 1.0f)/RAND_MAX); 
				break;
			}
			case SPIRAL:
			{
				// This is for spiral galaxies
				fRadius = m_RadialScaling*NormalGuassianNumber();
				fTheta = -1.0f*abs(fRadius)/m_SpiralTurnsFactor;
				
				// If we are inside the first complete turn of the spiral, we want
				// random elliptical distribution of points. If we are outside, we 
				// want a spiral distribution of points.
				if (-2.0f*D3DX_PI < fTheta)
				{
					fTheta = 2.0f*D3DX_PI*(((float)rand() + 1.0f)/RAND_MAX);
					fRadialSpread = m_RadialSpreadScaling;
				}
				else
				{
					// Now adjust the radius slightly to "spread" the spiral arms
					fRadialSpread = m_RadialSpreadScaling*NormalGuassianNumber();
				}

				fRadius += fRadialSpread;
				break;
			}
		}

		unsigned short usBrightness = (unsigned short)(255*(((float)rand() + 1.0f)/RAND_MAX));
		if (usBrightness > 255)
		{
			usBrightness = 255;
		}

		m_RadiusYThetaBrightness[i].x = fRadius;
		m_RadiusYThetaBrightness[i].y = m_VerticalSpread*exp(-m_VerticalExpFactor*fRadius*fRadius)*(2.0f*(((float)rand() + 1.0f)/RAND_MAX) - 1.0f);
		m_RadiusYThetaBrightness[i].z = fTheta;
		m_RadiusYThetaBrightness[i].w = (float)usBrightness;
	}
}

void Galaxy::UpdateGalaxy()
{
	if (m_fFixedCameraPostion == FALSE)
	{
		// rotate the world transform
		m_RelativeRotationAngle += 2.0f*D3DX_PI/5000.0f;
		if (m_RelativeRotationAngle > 2.0f*D3DX_PI)
		{
			m_RelativeRotationAngle -= 2.0f*D3DX_PI;
		}
		m_transform.RotateAbs(m_StartingXRotation + D3DX_PI/4.0f*sin(m_RelativeRotationAngle), 0.0f, 0.0f);
		m_pD3DDevice->SetTransform(D3DTS_WORLD, m_transform.GetTransform());
	}

    // Create the new vertices
    CUSTOMVERTEX* pVerts = new CUSTOMVERTEX[m_nNumberOfStars];
    for ( int i = 0; i < m_nNumberOfStars; i++ )
    {
		m_RadiusYThetaBrightness[i].z += m_RotationRate*0.01f;
		if (m_RadiusYThetaBrightness[i].z > 2.0f*D3DX_PI)
		{
			m_RadiusYThetaBrightness[i].z -= 2.0f*D3DX_PI;
		}

		unsigned short usBrightness = (unsigned short)m_RadiusYThetaBrightness[i].w;
		unsigned short usRed = (i % 55 == 0  && usBrightness > 100 ? 255 : usBrightness);
		unsigned short usGreen = (i % 77 == 0 && usBrightness > 100 ? 255 : usBrightness);
		unsigned short usBlue = (i % 99 == 0 && usBrightness > 100 ? 255 : usBrightness);

        ZeroMemory( &pVerts[i], sizeof( CUSTOMVERTEX ) );

		pVerts[i].x = m_RadiusYThetaBrightness[i].x*cos(m_RadiusYThetaBrightness[i].z);
        pVerts[i].y = m_RadiusYThetaBrightness[i].y;
        pVerts[i].z = m_RadiusYThetaBrightness[i].x*sin(m_RadiusYThetaBrightness[i].z);
        pVerts[i].color = D3DCOLOR_XRGB( usRed, usGreen, usBlue );
    }
    // Fill up the buffer with the new vertices
    m_VB.SetData(m_nNumberOfStars, pVerts);
	delete[] pVerts;
}

void Galaxy::RenderGalaxy()
{
    m_pD3DDevice->Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0, 0, 0 ), 1.0f, 0 ); 
    m_pD3DDevice->BeginScene();

    m_VB.Render(m_pD3DDevice, m_nNumberOfStars, D3DPT_POINTLIST );

    m_pD3DDevice->EndScene();
    m_pD3DDevice->Present( 0, 0, 0, 0 );
}

void Galaxy::ResetDevice()
{
    // Create the dynamic vertex buffer (will auto release if it already exists)
	m_VB.CreateBuffer
		( 
			m_pD3DDevice, 
			m_nNumberOfStars, 
			D3DFVF_XYZ | D3DFVF_DIFFUSE, 
			sizeof(CUSTOMVERTEX), 
			TRUE
		);

    // World transform
	m_transform = CWorldTransform();
    m_pD3DDevice->SetTransform(D3DTS_WORLD, m_transform.GetTransform());

	// View transform - first work out the viewing location.
	float CameraPosY = 0.0f;
	float CameraPosZ = m_FixedViewingDistance;
	if (m_fFixedCameraPostion == TRUE)
	{
		CameraPosY = m_FixedViewingDistance*sin(m_ViewingAngle*D3DX_PI/180.0f);
		CameraPosZ = m_FixedViewingDistance*cos(m_ViewingAngle*D3DX_PI/180.0f);
	}

    D3DXVECTOR3 cameraPosition(0.0f, CameraPosY, -1.0f*CameraPosZ);
    D3DXVECTOR3 cameraTarget(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 cameraUp(0.0f, 1.0f, 0.0f);
    D3DXMATRIX viewMatrix;
    D3DXMatrixLookAtLH( &viewMatrix, &cameraPosition, &cameraTarget, &cameraUp );
    m_pD3DDevice->SetTransform( D3DTS_VIEW, &viewMatrix );

	// Projection transform
    D3DXMATRIX projection;
    float aspect = (float)m_nWidth / (float)m_nHeight;
    D3DXMatrixPerspectiveFovLH(&projection, D3DX_PI / 3.0f, aspect, 0.1f, 1000.0f );
    m_pD3DDevice->SetTransform( D3DTS_PROJECTION, &projection );

    // Set render states
    m_pD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );      
    m_pD3DDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );  
    m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    m_pD3DDevice->SetRenderState( D3DRS_POINTSIZE, *((DWORD*)&m_SizeOfStars) );

	m_StartingXRotation = m_transform.GetXRotation();
}

///////////////////////////////////////////
// DirectX and windows related functions for rendering.
///////////////////////////////////////////
void Galaxy::SetupWndandTimer(HWND& hWnd, UINT_PTR timer)
{
	RECT rect;
	GetClientRect(hWnd, &rect);

	m_hWnd = hWnd;
	m_nWidth = rect.right;		
	m_nHeight = rect.bottom;

	// If we have supplied another timer, use this instead of creating
	// a new one (useful for dialogs)
	if (timer != NULL)
	{
		m_Timer = timer;
	}
	else
	{
		// Set timer to tick every 10 ms
		SetTimer(hWnd, m_Timer, 10, NULL);
	}
}

BOOL Galaxy::InitDirectX()
{
	// Create Direct3D Object
	m_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_pD3D9)
	{
		m_fD3DInitialised = false;
		return FALSE;
	}

	// Get display mode
	m_pD3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &m_displayMode);

	// Check for hardware T&L
	D3DCAPS9 D3DCaps;
	m_pD3D9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &D3DCaps);
	DWORD vertexProcessing = 0;
	if (D3DCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		vertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		// Check for pure device
		if (D3DCaps.DevCaps & D3DDEVCAPS_PUREDEVICE)
		{
			vertexProcessing |= D3DCREATE_PUREDEVICE;
		}
	}
	else
	{
		vertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	// Fill out the presentation parameters
	if (!BuildPresentationParameters())
	{
		m_fD3DInitialised = false;
		return FALSE;
	}

	// Create the device
	if ( FAILED( m_pD3D9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd, vertexProcessing, &m_D3Dpp, &m_pD3DDevice ) ) )
	{
		m_fD3DInitialised = false;
		return FALSE;
	}

	m_fD3DInitialised = true;
	return TRUE;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose:
Builds the D3DPRESENT_PARAMETERS structure using the current window size.
Returns: TRUE on success. FALSE if a valid depth format cannot be found.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
BOOL Galaxy::BuildPresentationParameters()
{
	ZeroMemory(&m_D3Dpp, sizeof(m_D3Dpp));
	D3DFORMAT adapterFormat = m_displayMode.Format;
	if ( SUCCEEDED( m_pD3D9->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8 ) ) )
	{
		m_D3Dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	}
	else if ( SUCCEEDED( m_pD3D9->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8 ) ) )
	{
		m_D3Dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	}
	else if ( SUCCEEDED( m_pD3D9->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D16 ) ) )
	{
		m_D3Dpp.AutoDepthStencilFormat = D3DFMT_D16;
	}
	else
	{
		return false;
	}

	m_D3Dpp.BackBufferWidth            = 0;
	m_D3Dpp.BackBufferHeight           = 0;
	m_D3Dpp.BackBufferFormat           = adapterFormat;
	m_D3Dpp.BackBufferCount            = 1;
	m_D3Dpp.MultiSampleType            = D3DMULTISAMPLE_NONE;
	m_D3Dpp.MultiSampleQuality         = 0;
	m_D3Dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
	m_D3Dpp.hDeviceWindow              = m_hWnd;
	m_D3Dpp.Windowed                   = TRUE;
	m_D3Dpp.EnableAutoDepthStencil     = TRUE;
	m_D3Dpp.FullScreen_RefreshRateInHz = 0;
	m_D3Dpp.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;

	return TRUE;
}

void Galaxy::Release()
{
	// Kill the windows timer
	if (m_Timer != NULL)
	{
		KillTimer(m_hWnd, m_Timer);
	}

	// Clean up resources
	if (m_RadiusYThetaBrightness != NULL)
	{
		delete[] m_RadiusYThetaBrightness;
		m_RadiusYThetaBrightness = NULL;
	}
	m_VB.Release();
	SAFE_RELEASE(m_pD3DDevice);
	SAFE_RELEASE(m_pD3D9);
	m_fD3DInitialised = false;
}

void Galaxy::GetRegistryConfig(HWND hDlg)
{
	HKEY key;
	if
	(
		RegOpenKeyEx
			( 
				HKEY_CURRENT_USER,
    			"Software\\Galaxy Simulation", //lpctstr
    			0,			//reserved
    			KEY_QUERY_VALUE,
    			&key
			) == ERROR_SUCCESS
	)	
	{
    	// Get the Galaxy Type
    	DWORD dsize = sizeof(m_GalaxyType);
    	DWORD dwtype =  0;
		RegQueryValueEx
			(
				key,
				"Galaxy Type", 
				NULL, 
				&dwtype, 
    			(BYTE*)&m_GalaxyType, 
				&dsize
			);

    	// Get the number of stars
    	dsize = sizeof(m_nNumberOfStars);
    	dwtype =  0;
		RegQueryValueEx
			(
				key,
				"Number Of Stars", 
				NULL, 
				&dwtype, 
    			(BYTE*)&m_nNumberOfStars, 
				&dsize
			);

		// Get the elliptical vertical spread factor
    	dsize = sizeof(m_VerticalSpread);
    	dwtype =  0;
		RegQueryValueEx
			(
				key,
				"Ellip Vert Spread", 
				NULL, 
				&dwtype, 
    			(BYTE*)&m_VerticalSpread, 
				&dsize
			);

		// Get the elliptical vertical Exp factorr
    	dsize = sizeof(m_VerticalExpFactor);
    	dwtype =  0;
		RegQueryValueEx
			(
				key,
				"Ellip Vert Exp Factor", 
				NULL, 
				&dwtype, 
    			(BYTE*)&m_VerticalExpFactor, 
				&dsize
			);

		// Get the radial spread
    	dsize = sizeof(m_RadialSpread);
    	dwtype =  0;
		RegQueryValueEx
			(
				key,
				"Radial Spread", 
				NULL, 
				&dwtype, 
    			(BYTE*)&m_RadialSpread, 
				&dsize
			);

		// Get the rotation rate
    	dsize = sizeof(m_RotationRate);
    	dwtype =  0;
		RegQueryValueEx
			(
				key,
				"Rotation Rate", 
				NULL, 
				&dwtype, 
    			(BYTE*)&m_RotationRate, 
				&dsize
			);

		// Get the size of stars
    	dsize = sizeof(m_SizeOfStars);
    	dwtype =  0;
		RegQueryValueEx
			(
				key,
				"Size Of Stars", 
				NULL, 
				&dwtype, 
    			(BYTE*)&m_SizeOfStars, 
				&dsize
			);

		// Get the fixed camera position
    	dsize = sizeof(m_fFixedCameraPostion);
    	dwtype =  0;
		RegQueryValueEx
			(
				key,
				"Fixed Camera Position", 
				NULL, 
				&dwtype, 
    			(BYTE*)&m_fFixedCameraPostion, 
				&dsize
			);

		// Get the viewing angle
    	dsize = sizeof(m_ViewingAngle);
    	dwtype =  0;
		RegQueryValueEx
			(
				key,
				"Viewing Angle", 
				NULL, 
				&dwtype, 
    			(BYTE*)&m_ViewingAngle, 
				&dsize
			);

		// Get the spiral turns factor
    	dsize = sizeof(m_SpiralTurnsFactor);
    	dwtype =  0;
		RegQueryValueEx
			(
				key,
				"Spiral Turns Factor", 
				NULL, 
				&dwtype, 
    			(BYTE*)&m_SpiralTurnsFactor, 
				&dsize
			);

		// Get the spiral radial scaling
    	dsize = sizeof(m_RadialScaling);
    	dwtype =  0;
		RegQueryValueEx
			(
				key,
				"Spiral Radial Scaling", 
				NULL, 
				&dwtype, 
    			(BYTE*)&m_RadialScaling, 
				&dsize
			);

		// Get the spiral radial spread scaling
    	dsize = sizeof(m_RadialSpreadScaling);
    	dwtype =  0;
		RegQueryValueEx
			(
				key,
				"Spiral Radial Spread Scaling", 
				NULL, 
				&dwtype, 
    			(BYTE*)&m_RadialSpreadScaling, 
				&dsize
			);

    	//Finished with key
    	RegCloseKey(key);
	}

	if (hDlg != NULL)
	{
		SetDialogValues(hDlg);
	}
}

void Galaxy::SetRegistryConfig(HWND hDlg)
{
	// First parse all dialog controls and get their values
	// into the galaxy class.
	if (hDlg != NULL)
	{
		GetDialogValues(hDlg);
	}

	// Now set all the values in the registry from the control.
    HKEY key;
    DWORD lpdw;

    if 
	(
		RegCreateKeyEx
			( 
				HKEY_CURRENT_USER,
    			"Software\\Galaxy Simulation", //lpctstr
    			0,			//reserved
    			"",			//ptr to null-term string specifying the object type of this key
    			REG_OPTION_NON_VOLATILE,
    			KEY_WRITE,
    			NULL,
    			&key,
    			&lpdw
			) == ERROR_SUCCESS
	)	
    {
		// Set the Galaxy Type
    	RegSetValueEx
			(
				key,
				"Galaxy Type", 
				0, 
				REG_DWORD, 
    			(BYTE*)&m_GalaxyType, 
				sizeof(m_GalaxyType)
			);

		// Set the number of stars
    	RegSetValueEx
			(
				key,
				"Number Of Stars", 
				0, 
				REG_DWORD, 
				(BYTE*)&m_nNumberOfStars, 
				sizeof(m_nNumberOfStars)
			);

		// Set the elliptical vertical spread factor
    	RegSetValueEx
			(
				key,
				"Ellip Vert Spread", 
				0, 
				REG_DWORD, 
    			(BYTE*)&m_VerticalSpread, 
				sizeof(m_VerticalSpread)
			);

		// Set the elliptical vertical Exp factor
    	RegSetValueEx
			(
				key,
				"Ellip Vert Exp Factor", 
				0, 
				REG_DWORD, 
    			(BYTE*)&m_VerticalExpFactor, 
				sizeof(m_VerticalExpFactor)
			);

		// Set the radial spread
    	RegSetValueEx
			(
				key,
				"Radial Spread", 
				0, 
				REG_DWORD, 
				(BYTE*)&m_RadialSpread, 
				sizeof(m_RadialSpread)
			);

		// Set the rotation rate
    	RegSetValueEx
			(
				key,
				"Rotation Rate", 
				0, 
				REG_DWORD, 
				(BYTE*)&m_RotationRate, 
				sizeof(m_RotationRate)
			);

		// Set the size of stars
    	RegSetValueEx
			(
				key,
				"Size Of Stars", 
				0, 
				REG_DWORD, 
				(BYTE*)&m_SizeOfStars, 
				sizeof(m_SizeOfStars)
			);

		// Set the fixed camera position
    	RegSetValueEx
			(
				key,
				"Fixed Camera Position", 
				0, 
				REG_DWORD, 
				(BYTE*)&m_fFixedCameraPostion, 
				sizeof(m_fFixedCameraPostion)
			);

		// Set the viewing angle
    	RegSetValueEx
			(
				key,
				"Viewing Angle", 
				0, 
				REG_DWORD, 
				(BYTE*)&m_ViewingAngle, 
				sizeof(m_ViewingAngle)
			);

		// Set the spiral turns factor
    	RegSetValueEx
			(
				key,
				"Spiral Turns Factor", 
				0, 
				REG_DWORD, 
    			(BYTE*)&m_SpiralTurnsFactor, 
				sizeof(m_SpiralTurnsFactor)
			);

		// Set the spiral radial scaling
    	RegSetValueEx
			(
				key,
				"Spiral Radial Scaling", 
				0, 
				REG_DWORD, 
				(BYTE*)&m_RadialScaling, 
				sizeof(m_RadialScaling)
			);

		// Set the spiral radial spread scaling
    	RegSetValueEx
			(
				key,
				"Spiral Radial Spread Scaling", 
				0, 
				REG_DWORD, 
				(BYTE*)&m_RadialSpreadScaling, 
				sizeof(m_RadialSpreadScaling)
			);

    	//Finished with keys
    	RegCloseKey(key);
    }
}

void Galaxy::GetDialogValues(HWND hDlg)
{
	if (IsDlgButtonChecked(hDlg, IDC_GALAXY_TYPE_ELLIPTICAL) == BST_CHECKED)
	{
		m_GalaxyType = ELLIPTICAL;
	}

	if (IsDlgButtonChecked(hDlg, IDC_GALAXY_TYPE_SPIRAL) == BST_CHECKED)
	{
		m_GalaxyType = SPIRAL;
	}

	HWND dialogControlHWnd;
	
	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_NUMBER_OF_STARS);				
	CString strNumberOfStars;
	GetWindowText(dialogControlHWnd, strNumberOfStars.GetBuffer(), 8);
	int nTempNoStars = atoi(strNumberOfStars);
	if (nTempNoStars <= 0)
	{
		nTempNoStars = 1;
	}
	m_nNumberOfStars = nTempNoStars;

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_VERT_SPREAD);				
	CString strVertSpread;
	GetWindowText(dialogControlHWnd, strVertSpread.GetBuffer(), 8);
	m_VerticalSpread = (float)atof(strVertSpread);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_EXP_FACTOR);				
	CString strExpFactor;
	GetWindowText(dialogControlHWnd, strExpFactor.GetBuffer(), 8);
	m_VerticalExpFactor = (float)atof(strExpFactor);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_RADIAL_SPREAD);				
	CString strRadialSpread;
	GetWindowText(dialogControlHWnd, strRadialSpread.GetBuffer(), 8);
	m_RadialSpread = (float)atof(strRadialSpread);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_ROTATION_RATE);				
	CString strRotationRate;
	GetWindowText(dialogControlHWnd, strRotationRate.GetBuffer(), 8);
	m_RotationRate = (float)atof(strRotationRate);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_SIZE_OF_STARS);				
	CString strSizeOfStars;
	GetWindowText(dialogControlHWnd, strSizeOfStars.GetBuffer(), 8);
	m_SizeOfStars = (float)atof(strSizeOfStars);

	if (IsDlgButtonChecked(hDlg, IDC_CHECK_FIXED_CAMERA) == BST_CHECKED)
	{
		m_fFixedCameraPostion = TRUE;
	}

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_VIEWING_ANGLE);				
	CString strViewingAngle;
	GetWindowText(dialogControlHWnd, strViewingAngle.GetBuffer(), 8);
	m_ViewingAngle = (float)atof(strViewingAngle);
	if (m_ViewingAngle > 90.0f)
	{
		m_ViewingAngle = 90.0f;
	}
	else if (m_ViewingAngle < -90.0f)
	{
		m_ViewingAngle = -90.0f;
	}

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_SPIRAL_TURNS_FACTOR);				
	CString strSpiralTurnsFactor;
	GetWindowText(dialogControlHWnd, strSpiralTurnsFactor.GetBuffer(), 8);
	m_SpiralTurnsFactor = (float)atof(strSpiralTurnsFactor);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_SPIRAL_RADIAL_SCALING);				
	CString strSpiralRadialScaling;
	GetWindowText(dialogControlHWnd, strSpiralRadialScaling.GetBuffer(), 8);
	m_RadialScaling = (float)atof(strSpiralRadialScaling);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_SPIRAL_RADIAL_SPREAD_SCALING);				
	CString strSpiralRadialSpreadScaling;
	GetWindowText(dialogControlHWnd, strSpiralRadialSpreadScaling.GetBuffer(), 8);
	m_RadialSpreadScaling = (float)atof(strSpiralRadialSpreadScaling);
}

void Galaxy::SetDialogValues(HWND hDlg)
{
	// We are setting up the config dialog so get entries
	// and enable appropriate controls.
	HWND dialogControlHWnd;
	
	switch (m_GalaxyType)
	{
		case ELLIPTICAL:
			dialogControlHWnd = GetDlgItem(hDlg, IDC_GALAXY_TYPE_ELLIPTICAL);
			SendMessage(dialogControlHWnd, BM_SETCHECK, BST_CHECKED, 0 );
			dialogControlHWnd = GetDlgItem(hDlg, IDC_GALAXY_TYPE_SPIRAL);
			SendMessage(dialogControlHWnd, BM_SETCHECK, BST_UNCHECKED, 0 );
			break;
		case SPIRAL:
			dialogControlHWnd = GetDlgItem(hDlg, IDC_GALAXY_TYPE_SPIRAL);
			SendMessage(dialogControlHWnd, BM_SETCHECK, BST_CHECKED, 0 );
			dialogControlHWnd = GetDlgItem(hDlg, IDC_GALAXY_TYPE_ELLIPTICAL);
			SendMessage(dialogControlHWnd, BM_SETCHECK, BST_UNCHECKED, 0 );
			break;
	}

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_NUMBER_OF_STARS);
	CString strNumberOfStars;
	strNumberOfStars.Format("%d", m_nNumberOfStars);
	SetWindowText(dialogControlHWnd, strNumberOfStars);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_VERT_SPREAD);
	CString strVertSpread;
	strVertSpread.Format("%f", m_VerticalSpread);
	SetWindowText(dialogControlHWnd, strVertSpread);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_EXP_FACTOR);
	CString strExpFactor;
	strExpFactor.Format("%f", m_VerticalExpFactor);
	SetWindowText(dialogControlHWnd, strExpFactor);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_RADIAL_SPREAD);
	CString strRadialSpread;
	strRadialSpread.Format("%f", m_RadialSpread);
	SetWindowText(dialogControlHWnd, strRadialSpread);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_ROTATION_RATE);
	CString strRotationRate;
	strRotationRate.Format("%f", m_RotationRate);
	SetWindowText(dialogControlHWnd, strRotationRate);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_SIZE_OF_STARS);
	CString strSizeOfStars;
	strSizeOfStars.Format("%f", m_SizeOfStars);
	SetWindowText(dialogControlHWnd, strSizeOfStars);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_CHECK_FIXED_CAMERA);
	SendMessage(dialogControlHWnd, BM_SETCHECK, m_fFixedCameraPostion ? BST_CHECKED : BST_UNCHECKED, 0);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_VIEWING_ANGLE);
	CString strViewingAngle;
	strViewingAngle.Format("%f", m_ViewingAngle);
	SetWindowText(dialogControlHWnd, strViewingAngle);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_SPIRAL_TURNS_FACTOR);
	CString strSpiralTurnsFactor;
	strSpiralTurnsFactor.Format("%f", m_SpiralTurnsFactor);
	SetWindowText(dialogControlHWnd, strSpiralTurnsFactor);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_SPIRAL_RADIAL_SCALING);
	CString strSpiralRadialScaling;
	strSpiralRadialScaling.Format("%f", m_RadialScaling);
	SetWindowText(dialogControlHWnd, strSpiralRadialScaling);

	dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_SPIRAL_RADIAL_SPREAD_SCALING);
	CString strSpiralRadialSpreadScaling;
	strSpiralRadialSpreadScaling.Format("%f", m_RadialSpreadScaling);
	SetWindowText(dialogControlHWnd, strSpiralRadialSpreadScaling);
}