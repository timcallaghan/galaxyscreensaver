// Main file for the Galaxy Simulation Screensaver.
// In this file we provide the three required callback functions for the screensaver library 
// i.e. ScreenSaverProc, ScreenSaverConfigureDialog and RegisterDialogClasses
// All other functionality (i.e. D3D initialisation and rendering) occurs in the Galaxy class.

#include "DX/include/stdafx.h"
#include <scrnsave.h>
#include "Galaxy.h"
#include "resource.h"

// Global galaxy instance.
Galaxy g_Galaxy;

LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch ( message ) 
	{
		case WM_CREATE:
			// Setup base window and timer info then init directX
			g_Galaxy.SetupWndandTimer(hWnd);
			if (g_Galaxy.InitDirectX())
			{
				// Get the galaxy parameters from the registry 
				// and set them up (if they exist)
				g_Galaxy.GetRegistryConfig();

				// Reset the device for rendering galaxies and initialise the stars.
				g_Galaxy.ResetDevice();
				g_Galaxy.Initialize();
			}
			return 0;
	    
		case WM_DESTROY:
			// Kill the timer and release all resources.
			g_Galaxy.Release();
			return 0;

		case WM_TIMER:
			if (g_Galaxy.m_fD3DInitialised)
			{
				// Animate our galaxy
				g_Galaxy.UpdateGalaxy();
				g_Galaxy.RenderGalaxy();
			}
			return 0;				
    }

	// Let the screensaver library handle any messages we don't care about.
	return DefScreenSaverProc(hWnd, message, wParam, lParam);
}

BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) 
    {
		case WM_INITDIALOG:
			{
				// We need to associate the timer with the main dlg window
				// because the message loop will only process messages for
				// this window (and our preview window is a child of this window
				// with no message loop of its own etc.) Note that g_Galaxy.Release()
				// will kill the timer when we respond to IDOK or IDCANCEL
				UINT_PTR timerForMainDlg = NULL; 
				SetTimer(hDlg, timerForMainDlg, 10, NULL);

				// Release any resources in case they have been created (should never happen I think)
				if (g_Galaxy.m_pD3DDevice != NULL)
				{
					g_Galaxy.Release();
				}

				// Now perform initialisation and pass in the timer of the main dlg
				// to override the timer creation code path.
				HWND dialogControlHWND = GetDlgItem(hDlg, IDC_PREVIEW_PANE);
				g_Galaxy.SetupWndandTimer(dialogControlHWND, timerForMainDlg);

				if (g_Galaxy.InitDirectX())
				{
					// Get our registry values if they exist and populate the dialog controls.
    				g_Galaxy.GetRegistryConfig(hDlg);

					// D3D reset and then initialise our galaxy stars.
					g_Galaxy.ResetDevice();
					g_Galaxy.Initialize();
				}
			}
			return TRUE;

		case WM_TIMER:
			{
				if (g_Galaxy.m_fD3DInitialised)
				{
					g_Galaxy.UpdateGalaxy();
					g_Galaxy.RenderGalaxy();
				}
			}
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) 
			{
				case IDC_GALAXY_TYPE_ELLIPTICAL:
					if (IsDlgButtonChecked(hDlg, IDC_GALAXY_TYPE_ELLIPTICAL) == BST_CHECKED)
					{
						HWND dialogControlHWnd = GetDlgItem(hDlg, IDC_GALAXY_TYPE_SPIRAL);
						SendMessage(dialogControlHWnd, BM_SETCHECK, BST_UNCHECKED, 0 );
						g_Galaxy.m_GalaxyType = Galaxy::ELLIPTICAL;
						g_Galaxy.Initialize();
					}
    				return TRUE;

				case IDC_GALAXY_TYPE_SPIRAL:
					if (IsDlgButtonChecked(hDlg, IDC_GALAXY_TYPE_SPIRAL) == BST_CHECKED)
					{
						HWND dialogControlHWnd = GetDlgItem(hDlg, IDC_GALAXY_TYPE_ELLIPTICAL);
						SendMessage(dialogControlHWnd, BM_SETCHECK, BST_UNCHECKED, 0 );
						g_Galaxy.m_GalaxyType = Galaxy::SPIRAL;
						g_Galaxy.Initialize();
					}
    				return TRUE;

				case IDC_BTN_RESET_DEFAULTS:
					{
						g_Galaxy.ResetDefaults(hDlg);
						g_Galaxy.ResetDevice();
						g_Galaxy.Initialize();
					}
					return TRUE;

				case IDC_EDIT_NUMBER_OF_STARS:
					{
						HWND dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_NUMBER_OF_STARS);				
						CString strNumberOfStars;
						GetWindowText(dialogControlHWnd, strNumberOfStars.GetBuffer(), 8);
						int nTempNoStars = atoi(strNumberOfStars);
						if (nTempNoStars <= 0)
						{
							nTempNoStars = 1;
							CString strNumberOfStars;
							strNumberOfStars.Format("%d", nTempNoStars);
							SetWindowText(dialogControlHWnd, strNumberOfStars);
						}
						g_Galaxy.m_nNumberOfStars = nTempNoStars;
						g_Galaxy.ResetDevice();
						g_Galaxy.Initialize();
					}
					return TRUE;

				case IDC_EDIT_VERT_SPREAD:
					{
						HWND dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_VERT_SPREAD);				
						CString strVertSpread;
						GetWindowText(dialogControlHWnd, strVertSpread.GetBuffer(), 8);
						g_Galaxy.m_VerticalSpread = (float)atof(strVertSpread);
						g_Galaxy.Initialize();
					}
					return TRUE;

				case IDC_EDIT_EXP_FACTOR:
					{
						HWND dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_EXP_FACTOR);				
						CString strExpFactor;
						GetWindowText(dialogControlHWnd, strExpFactor.GetBuffer(), 8);
						g_Galaxy.m_VerticalExpFactor = (float)atof(strExpFactor);
						g_Galaxy.Initialize();
					}
					return TRUE;

				case IDC_EDIT_RADIAL_SPREAD:
					{
						HWND dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_RADIAL_SPREAD);				
						CString strRadialSpread;
						GetWindowText(dialogControlHWnd, strRadialSpread.GetBuffer(), 8);
						g_Galaxy.m_RadialSpread = (float)atof(strRadialSpread);
						g_Galaxy.Initialize();
					}
					return TRUE;

				case IDC_EDIT_ROTATION_RATE:
					{
						HWND dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_ROTATION_RATE);				
						CString strRotationRate;
						GetWindowText(dialogControlHWnd, strRotationRate.GetBuffer(), 8);
						g_Galaxy.m_RotationRate = (float)atof(strRotationRate);
						g_Galaxy.Initialize();
					}
					return TRUE;

				case IDC_EDIT_SIZE_OF_STARS:
					{
						HWND dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_SIZE_OF_STARS);				
						CString strSizeOfStars;
						GetWindowText(dialogControlHWnd, strSizeOfStars.GetBuffer(), 8);
						g_Galaxy.m_SizeOfStars = (float)atof(strSizeOfStars);
						g_Galaxy.Initialize();
					}
					return TRUE;

				case IDC_CHECK_FIXED_CAMERA:
					{
						if (IsDlgButtonChecked(hDlg, IDC_CHECK_FIXED_CAMERA) == BST_CHECKED)
						{
							g_Galaxy.m_fFixedCameraPostion = TRUE;
						}
						else
						{
							g_Galaxy.m_fFixedCameraPostion = FALSE;
							g_Galaxy.m_RelativeRotationAngle = 0.0f;
						}
						g_Galaxy.ResetDevice();
						g_Galaxy.Initialize();
					}
					return TRUE;

				case IDC_EDIT_VIEWING_ANGLE:
					{
						HWND dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_VIEWING_ANGLE);
						CString strViewingAngle;
						GetWindowText(dialogControlHWnd, strViewingAngle.GetBuffer(), 8);
						float tempViewingAngle = (float)atof(strViewingAngle);
						if (tempViewingAngle > 90.0f)
						{
							tempViewingAngle = 90.0f;
							strViewingAngle.Format("%f", tempViewingAngle);
							SetWindowText(dialogControlHWnd, strViewingAngle);
						}
						else if (tempViewingAngle < -90.0f)
						{
							tempViewingAngle = -90.0f;
							strViewingAngle.Format("%f", tempViewingAngle);
							SetWindowText(dialogControlHWnd, strViewingAngle);
						}

						g_Galaxy.m_ViewingAngle = tempViewingAngle;
						if (g_Galaxy.m_fFixedCameraPostion == TRUE)
						{
							g_Galaxy.ResetDevice();
							g_Galaxy.Initialize();
						}
					}
					return TRUE;

				case IDC_EDIT_SPIRAL_TURNS_FACTOR:
					{
						HWND dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_SPIRAL_TURNS_FACTOR);				
						CString strSpiralTurnsFactor;
						GetWindowText(dialogControlHWnd, strSpiralTurnsFactor.GetBuffer(), 8);
						g_Galaxy.m_SpiralTurnsFactor = (float)atof(strSpiralTurnsFactor);
						g_Galaxy.Initialize();
					}
					return TRUE;

				case IDC_EDIT_SPIRAL_RADIAL_SCALING:
					{
						HWND dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_SPIRAL_RADIAL_SCALING);				
						CString strSpiralRadialScaling;
						GetWindowText(dialogControlHWnd, strSpiralRadialScaling.GetBuffer(), 8);
						g_Galaxy.m_RadialScaling = (float)atof(strSpiralRadialScaling);
						g_Galaxy.Initialize();
					}
					return TRUE;

				case IDC_EDIT_SPIRAL_RADIAL_SPREAD_SCALING:
					{
						HWND dialogControlHWnd = GetDlgItem(hDlg, IDC_EDIT_SPIRAL_RADIAL_SPREAD_SCALING);				
						CString strSpiralRadialSpreadScaling;
						GetWindowText(dialogControlHWnd, strSpiralRadialSpreadScaling.GetBuffer(), 8);
						g_Galaxy.m_RadialSpreadScaling = (float)atof(strSpiralRadialSpreadScaling);
						g_Galaxy.Initialize();
					}
					return TRUE;
					
    			case IDOK:
					g_Galaxy.SetRegistryConfig(hDlg);
    				EndDialog( hDlg, LOWORD( wParam ) == IDOK );
					g_Galaxy.Release();
    				return TRUE; 

    			case IDCANCEL: 
    				EndDialog( hDlg, LOWORD( wParam ) == IDOK );
					g_Galaxy.Release();
    				return TRUE;   
			}

    }

    return FALSE; 
}

// We only need to do stuff in here if we are using custom dialog controls
// or special windows functionality.
BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
    return TRUE;
}
