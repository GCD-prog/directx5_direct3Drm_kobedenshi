#include <windows.h>

#include <ddraw.h>
#include <d3drmwin.h>
#include <mmsystem.h>

#include <stdio.h>
#include <math.h>

#define INITGUID

#define WAIT 25

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "d3drm.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

LPDIRECTDRAW lpDirectDraw;
LPDIRECTDRAWCLIPPER lpDDClipper;
LPDIRECTDRAWSURFACE lpPrimary;
LPDIRECTDRAWSURFACE lpBackbuffer;
LPDIRECTDRAWSURFACE lpZbuffer;

//Direct3D 関連
LPDIRECT3D lpDirect3D;
LPDIRECT3DDEVICE lpD3DDevice;

//Direct3D RM 関連
LPDIRECT3DRM lpDirect3DRM;
LPDIRECT3DRMDEVICE lpD3DRMDevice;
LPDIRECT3DRMVIEWPORT lpD3DRMView;
LPDIRECT3DRMFRAME lpD3DRMScene;
LPDIRECT3DRMFRAME lpD3DRMCamera;

LPDIRECT3DRMFRAME lpD3DRMFrame;

LPDIRECT3DRMFRAME lpLandFrame;

LPDIRECTDRAWSURFACE lpScreen = NULL;

char szDevice[128], szDDDeviceName[128] = "default";

void FrameCnt(void)
{
	     static int cnt;
		 static DWORD Nowtime,Prevtime;
		 static char text[10];
		 static char Data[256];

		 HDC hdc;
		 cnt++;
		 Nowtime=timeGetTime();
		 if((Nowtime-Prevtime)>=1000){
			 Prevtime=Nowtime;
			 wsprintf(text,"%d fps",cnt);
			 wsprintf(Data, "%s , [%s]", szDevice, szDDDeviceName);
			 cnt=0;
		 }

		 lpBackbuffer->GetDC(&hdc);
		 TextOut(hdc,0,0,text,strlen(text));
		 TextOut(hdc,100,0,Data,strlen(Data));
		 lpBackbuffer->ReleaseDC(hdc);
}


BOOL SetLight(void)
{
	//アンビエント光源を配置
	LPDIRECT3DRMLIGHT lpD3DRMLightAmbient;
	
	lpDirect3DRM->CreateLightRGB(D3DRMLIGHT_AMBIENT, D3DVAL(5.0), D3DVAL(5.0), D3DVAL(5.0), &lpD3DRMLightAmbient);
	lpD3DRMScene->AddLight(lpD3DRMLightAmbient);
	lpD3DRMLightAmbient->Release();

	LPDIRECT3DRMFRAME lpD3DRMLightFrame;
	lpDirect3DRM->CreateFrame(lpD3DRMScene, &lpD3DRMLightFrame);
	
	//ポイント光源を配置
	LPDIRECT3DRMLIGHT lpD3DRMLightPoint;

	lpDirect3DRM->CreateLightRGB(D3DRMLIGHT_POINT, D3DVAL(0.9), D3DVAL(0.9), D3DVAL(0.9), &lpD3DRMLightPoint);
	
	lpD3DRMLightFrame->SetPosition(lpD3DRMScene, D3DVAL(10.0), D3DVAL(0.0), D3DVAL(0.0));
	lpD3DRMLightFrame->AddLight(lpD3DRMLightPoint);

	lpD3DRMLightPoint->Release();

	lpD3DRMLightFrame->Release();

	return TRUE;
}

BOOL SetObject(void)
{
	//Direct3DRMMeshBuilderを作成
	LPDIRECT3DRMMESHBUILDER lpD3DRMMeshBuilder = NULL;

	lpDirect3DRM->CreateMeshBuilder(&lpD3DRMMeshBuilder);

	lpD3DRMMeshBuilder->Load("datafile\\test.x", NULL, D3DRMLOAD_FROMFILE, NULL, NULL);

	//Direct3DRMMeshBuilderのフレームを作成し、シーンに配置
	lpDirect3DRM->CreateFrame(lpD3DRMScene, &lpD3DRMFrame);
	lpD3DRMFrame->SetPosition(lpD3DRMScene, D3DVAL(0.0), D3DVAL(3.0), D3DVAL(30.0));
	lpD3DRMFrame->SetOrientation(lpD3DRMScene, D3DVAL(0.0), D3DVAL(0.0), D3DVAL(1.0), D3DVAL(0.0), D3DVAL(1), D3DVAL(0.0));

	// 呼び出したXファイルを回転
	lpD3DRMFrame->SetRotation(lpD3DRMScene,D3DVALUE(0.0),D3DVALUE(1),D3DVALUE(0.2),D3DVALUE(0.05));

	lpD3DRMFrame->AddVisual((LPDIRECT3DRMVISUAL)lpD3DRMMeshBuilder);

	lpD3DRMFrame->Release();
	lpD3DRMMeshBuilder->Release();

	return TRUE;
}


BOOL Landscape(void)
{
	LPDIRECT3DRMTEXTURE lpLandtex;
    
	lpDirect3DRM->LoadTexture("datafile\\Texture.bmp", &lpLandtex);
	
	const int max = 20;
	for (int z = 0; z < max; z++) {
		for (int x = 0; x < max; x++) {

			// 頂点の計算　左下(P0)、右下(P1)、左上(P2)、右上(P3)
			D3DVECTOR vPoint0, vPoint1, vPoint2, vPoint3;
			vPoint0.x =   D3DVAL(x) * 2.0f        - D3DVAL(max);
              vPoint0.y = 0.0f;
              vPoint0.z = D3DVAL(z) * 2.0f        - D3DVAL(max);
			vPoint1.x =   D3DVAL(x) * 2.0f + 2.0f - D3DVAL(max);
              vPoint1.y = 0.0f;
              vPoint1.z = D3DVAL(z) * 2.0f        - D3DVAL(max);
			vPoint2.x =   D3DVAL(x) * 2.0f        - D3DVAL(max);
              vPoint2.y = 0.0f;
              vPoint2.z = D3DVAL(z) * 2.0f + 2.0f - D3DVAL(max);
			vPoint3.x =   D3DVAL(x) * 2.0f + 2.0f - D3DVAL(max);
              vPoint3.y = 0.0f;
              vPoint3.z = D3DVAL(z) * 2.0f + 2.0f - D3DVAL(max);

			// テクスチャの頂点の計算
			D3DVALUE u1, v1, u2, v2;
			if (x % 2 == 0) {
				u1 = 0.0f; u2 = 0.5f;
			} else {
				u1 = 0.5f; u2 = 1.0f;
			}
			if (z % 2 == 0) {
				v1 = 0.0f; v2 = 0.5f;
			} else {
				v1 = 0.5f; v2 = 1.0f;
			}
			v1 = 1.0f - v1;	// ビットマップ形式のためＹ成分を逆転する
			v2 = 1.0f - v2;

			// 左下(P0)、右上(P3)、左上(P2)の頂点からポリゴンを作る
			LPDIRECT3DRMMESHBUILDER lpLand_mesh;

			lpDirect3DRM->CreateMeshBuilder(&lpLand_mesh);

				// 頂点を追加
                lpLand_mesh->AddVertex(vPoint0.x, vPoint0.y, vPoint0.z);
				lpLand_mesh->AddVertex(vPoint2.x, vPoint2.y, vPoint2.z);
				lpLand_mesh->AddVertex(vPoint3.x, vPoint3.y, vPoint3.z);
				lpLand_mesh->AddVertex(vPoint1.x, vPoint1.y, vPoint1.z);

				D3DVECTOR vNorm0, vNorm1;
				vNorm0.x = 0.0f; vNorm0.y = 1.0f; vNorm0.z = 0.0;
				vNorm1.x = 0.0f; vNorm1.y = 1.0f; vNorm1.z = 0.0f;

				// 法線ベクトルの追加
				lpLand_mesh->AddNormal(vNorm0.x, vNorm0.y, vNorm0.z);
				lpLand_mesh->AddNormal(vNorm1.x, vNorm1.y, vNorm1.z);

				// メッシュにポリゴンを追加
				LPDIRECT3DRMFACE face = NULL;
				lpLand_mesh->CreateFace(&face);
				face->SetTexture(lpLandtex);
				face->AddVertexAndNormalIndexed(0, 0);
				face->AddVertexAndNormalIndexed(2, 0);
				face->AddVertexAndNormalIndexed(3, 0);
				face->SetTextureCoordinates(0, u1, v1);
				face->SetTextureCoordinates(1, u2, v2);
				face->SetTextureCoordinates(2, u2, v1);
				lpLand_mesh->AddFace(face);
				face->Release();

			// 左下(P0)、右下(P1)、右上(P3)の頂点からポリゴンを作る
				face = NULL;
				lpLand_mesh->CreateFace(&face);
				face->SetTexture(lpLandtex);
				face->AddVertexAndNormalIndexed(0, 1);
				face->AddVertexAndNormalIndexed(1, 1);
				face->AddVertexAndNormalIndexed(2, 1);
				face->SetTextureCoordinates(0, u1, v1);
				face->SetTextureCoordinates(1, u1, v2);
				face->SetTextureCoordinates(2, u2, v2);
				lpLand_mesh->AddFace(face);
				face->Release();

			// 地面
			lpDirect3DRM->CreateFrame(lpD3DRMScene, &lpLandFrame);
			lpLandFrame->SetPosition(lpD3DRMScene, 0.0f, 0.0f, 0.0f);

            // メッシュを地面フレームに追加する
            lpLandFrame->AddVisual(lpLand_mesh);

			lpLandFrame->Release();

            lpLand_mesh->Release();

		}

	}

	// 不要なインタフェースを解放
	lpLandtex->Release();

	return TRUE;
}

BOOL RenderFrame(void)
{
	if ( lpPrimary->IsLost() == DDERR_SURFACELOST )		lpPrimary->Restore();

		RECT Scrrc={0,0,640,480};

		static DWORD nowTime, prevTime;
		nowTime = timeGetTime();
		if( (nowTime - prevTime)<1000/60 ) return 0;
		prevTime = nowTime;


		//Direct3DRM レンダリング処理
		lpD3DRMScene->Move(D3DVAL(1.0)); 
		lpD3DRMView->Clear();

		// 2d スプライト 描画
		lpBackbuffer->BltFast(0,0,lpScreen,&Scrrc,DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT);

		lpD3DRMView->Render(lpD3DRMScene);

		lpD3DRMDevice->Update();

		//FPS
		FrameCnt();

		// Flip()
		// lpPrimary->Flip(NULL, DDFLIP_WAIT);

		RECT rc;
		SetRect(&rc, 0, 0, 640, 480);

		lpPrimary->BltFast(0, 0, lpBackbuffer, NULL, DDBLTFAST_NOCOLORKEY);

		return TRUE;
}

void ReleaseAll(void)
{
	if(lpD3DRMCamera != NULL)
		lpD3DRMCamera->Release();
	if(lpD3DRMScene != NULL)
		lpD3DRMScene->Release();
	if(lpD3DRMView != NULL)
		lpD3DRMView->Release();
	if(lpD3DRMDevice != NULL)
		lpD3DRMDevice->Release();
	if(lpDirect3DRM != NULL)
		lpDirect3DRM->Release();

	if(lpDirect3D != NULL)
		lpDirect3D->Release();
	if(lpD3DDevice != NULL)
		lpD3DDevice->Release();

	if(lpZbuffer != NULL)
		lpZbuffer->Release();
	if(lpPrimary != NULL)
		lpPrimary->Release();
//	if(lpBackbuffer != NULL)
//		lpBackbuffer->Release();
	if(lpDDClipper != NULL)
		lpDDClipper->Release();
	if(lpDirectDraw != NULL)
		lpDirectDraw->Release();
}

GUID* D3D_GuidSearch(HWND hwnd)
{
	HRESULT d3dret;
	GUID*   Guid;
	LPDIRECT3D          lpD3D;
	LPDIRECTDRAW        lpDD;
	D3DFINDDEVICESEARCH S_DATA;
	D3DFINDDEVICERESULT R_DATA;
	char str[100];

	memset(&S_DATA, 0, sizeof S_DATA);
	S_DATA.dwSize = sizeof S_DATA;
	S_DATA.dwFlags = D3DFDS_COLORMODEL;
	S_DATA.dcmColorModel = D3DCOLOR_RGB;
	memset(&R_DATA, 0, sizeof R_DATA);
	R_DATA.dwSize = sizeof R_DATA;

	//DIRECTDRAWの生成
	d3dret = DirectDrawCreate(NULL, &lpDD, NULL);
	if (d3dret != DD_OK) {
		MessageBox( hwnd, "ダイレクトドローオブジェクトの生成に失敗しました", "初期化", MB_OK);
		lpDD->Release();
		return NULL;
	}

	//DIRECTD3Dの生成
	d3dret = lpDD->QueryInterface(IID_IDirect3D, (void**)&lpD3D);
	if (d3dret != D3D_OK) {
		MessageBox( hwnd, "ダイレクト３Ｄオブジェクトの生成に失敗しました", "初期化", MB_OK);
		lpDD->Release();
		lpD3D->Release();
		return NULL;
	}
	//デバイスの列挙
	d3dret = lpD3D->FindDevice(&S_DATA, &R_DATA);
	if (d3dret != D3D_OK) {
		MessageBox( hwnd, "デバイスの列挙に失敗しました", "初期化", MB_OK);
		lpDD->Release();
		lpD3D->Release();
		return NULL;
	}

	//ガイドの取得
	Guid = &R_DATA.guid;
	//不要になったオブジェクトのリリース
	lpDD->Release();
	lpD3D->Release();
	wsprintf(str, "%x", *Guid);
	return (Guid);
}

BOOL CALLBACK DDEnumCallback(GUID FAR* lpGUID, LPSTR lpDriverDesc, LPSTR lpDriverName, LPVOID lpContext)
{
	LPDIRECTDRAW lpDD;
	DDCAPS DriverCaps, HELCaps;

	if(DirectDrawCreate(lpGUID, &lpDD, NULL) != DD_OK) {
		*(LPDIRECTDRAW*)lpContext = NULL;
		return DDENUMRET_OK;
	}

	ZeroMemory(&DriverCaps, sizeof(DDCAPS));
	DriverCaps.dwSize = sizeof(DDCAPS);
	ZeroMemory(&HELCaps, sizeof(DDCAPS));
	HELCaps.dwSize = sizeof(DDCAPS);

	if(lpDD->GetCaps(&DriverCaps, &HELCaps) == DD_OK) {
		if ((DriverCaps.dwCaps & DDCAPS_3D) && (DriverCaps.ddsCaps.dwCaps & DDSCAPS_TEXTURE)) {
			*(LPDIRECTDRAW*)lpContext = lpDD;
			lstrcpy(szDDDeviceName, lpDriverDesc);
			return DDENUMRET_CANCEL;
		}
	}

	*(LPDIRECTDRAW*)lpContext = NULL;
	lpDD->Release();

	return DDENUMRET_OK;
}

LRESULT APIENTRY WndFunc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	switch(msg){

	case WM_CREATE:

		break;

	case WM_KEYDOWN:
				// Escキーでプログラムを終了します
		if(wParam==VK_ESCAPE){

			//画面モードを元に戻す
			lpDirectDraw->SetCooperativeLevel(hwnd, DDSCL_NORMAL);
			lpDirectDraw->RestoreDisplayMode();

			lpScreen->Release();

			ReleaseAll(); //各オブジェクトをReleaseAll()で解放する

			PostQuitMessage(0);

		}

		break;

	case WM_DESTROY:

			lpScreen->Release();

			ReleaseAll(); //各オブジェクトをReleaseAll()で解放する

			PostQuitMessage(0);

		break;

	default:
		return (DefWindowProc(hwnd, msg, wParam, lParam));
	}
	return 0;
}

void LoadBMP(LPDIRECTDRAWSURFACE lpSurface,char *fname)
{
	     HBITMAP hBmp=NULL;
		 BITMAP bm;
		 HDC hdc,hMemdc;

		 hBmp=(HBITMAP)LoadImage(GetModuleHandle(NULL),fname,IMAGE_BITMAP,0,0,
			      LR_CREATEDIBSECTION|LR_LOADFROMFILE);
		 GetObject(hBmp,sizeof(bm),&bm);

		 hMemdc=CreateCompatibleDC(NULL);
		 SelectObject(hMemdc,hBmp);

		 lpSurface->GetDC(&hdc);
		 BitBlt(hdc,0,0,bm.bmWidth,bm.bmHeight,hMemdc,0,0,SRCCOPY);
		 lpSurface->ReleaseDC(hdc);

		 DeleteDC(hMemdc);
		 DeleteObject(hBmp);
}

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrev,LPSTR lpszCmdParam,int nCmdShow)
{

		lpDirectDraw = NULL;
		lpDDClipper = NULL;
		lpPrimary = NULL;
		lpBackbuffer = NULL;
		lpZbuffer = NULL;
		lpDirect3DRM = NULL;
		lpD3DRMDevice = NULL;
		lpD3DRMView = NULL;
		lpD3DRMScene = NULL;

		MSG msg;

		HWND hwnd;

		DDSURFACEDESC Dds;
		DDSCAPS Ddscaps;

		WNDCLASS wc;
		char szAppName[] = "Generic Game SDK Window";
		
		wc.style = CS_DBLCLKS;
		wc.lpfnWndProc = WndFunc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.hIcon = LoadIcon(NULL,IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = szAppName;

		RegisterClass(&wc);

		hwnd=CreateWindowEx(
							WS_EX_TOPMOST,
							szAppName,
							"Direct X",
							WS_VISIBLE|WS_POPUP,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							640,480,
							NULL,NULL,hInst,
							NULL);

		if(!hwnd)return FALSE;

		ShowWindow(hwnd,nCmdShow);
		UpdateWindow(hwnd);
		SetFocus(hwnd);

		ShowCursor(FALSE); //カーソルを隠す

		//Direct3DRMの構築
		Direct3DRMCreate(&lpDirect3DRM);

		//DirectDrawClipperの構築
		DirectDrawCreateClipper(0, &lpDDClipper, NULL);

		// DirectDrawドライバを列挙する
		DirectDrawEnumerate(DDEnumCallback, &lpDirectDraw);

		// 列挙によってDirectDrawドライバを決める
		// 決定しなかった場合、現在アクティブなドライバを使う
		if(!lpDirectDraw){
			lstrcpy(szDDDeviceName, "Active Driver");
			DirectDrawCreate(NULL, &lpDirectDraw, NULL);
		}

		lpDirectDraw->SetCooperativeLevel(hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT);

		//ディスプレイモード変更
		lpDirectDraw->SetDisplayMode(640, 480, 16);

		//基本サーフェスとフロントバッファの生成 (１つを作成)
		ZeroMemory(&Dds, sizeof(DDSURFACEDESC));
		Dds.dwSize = sizeof(DDSURFACEDESC);
		Dds.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		Dds.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE;
		Dds.dwBackBufferCount = 1;

		lpDirectDraw->CreateSurface(&Dds,&lpPrimary,NULL);

		//バックバッファのポインタ取得
		Ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		lpPrimary->GetAttachedSurface(&Ddscaps,&lpBackbuffer);

		// Z-Buffer作成
		//基本サーフェスとバッファ１つを作成
		ZeroMemory(&Dds, sizeof(DDSURFACEDESC));
		Dds.dwSize = sizeof(DDSURFACEDESC);
		Dds.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_ZBUFFERBITDEPTH;
		Dds.dwHeight = 640;
		Dds.dwWidth = 480;
		Dds.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
		Dds.dwZBufferBitDepth = 16;

		lpDirectDraw->CreateSurface(&Dds,&lpZbuffer,NULL);

		lpBackbuffer->AddAttachedSurface(lpZbuffer);

/*
		//DirectDraw Clipper
		struct _MYRGNDATA {
			RGNDATAHEADER rdh;
			RECT rc;
		}rgndata;
	
		rgndata.rdh.dwSize = sizeof(RGNDATAHEADER);
		rgndata.rdh.iType = RDH_RECTANGLES;
		rgndata.rdh.nCount = 1;
		rgndata.rdh.nRgnSize = sizeof(RECT)*1;
		rgndata.rdh.rcBound.left = 0;
		rgndata.rdh.rcBound.right = 640;
		rgndata.rdh.rcBound.top = 0;
		rgndata.rdh.rcBound.bottom = 480;

		rgndata.rc.top = 0;
		rgndata.rc.bottom = 480;
		rgndata.rc.left = 0;
		rgndata.rc.right = 640;

		lpDirectDraw->CreateClipper(0, &lpDDClipper, NULL);
		lpDDClipper->SetClipList((LPRGNDATA)&rgndata, NULL);
		lpBackbuffer->SetClipper(lpDDClipper);
*/

		DDCOLORKEY          ddck;

		//背景サーフェスを作成
		Dds.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
		Dds.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
		Dds.dwWidth=640;
		Dds.dwHeight=480;
		lpDirectDraw->CreateSurface(&Dds,&lpScreen,NULL);

		//カラーキーの指定 白:RGB(255, 255, 255)　黒:RGB(0, 0, 0)
		ddck.dwColorSpaceLowValue=RGB(0, 0, 0);
		ddck.dwColorSpaceHighValue=RGB(0, 0, 0);
		lpScreen->SetColorKey(DDCKEY_SRCBLT,&ddck);

		//各サーフェスに画像を読み込む
		LoadBMP(lpScreen,"datafile\\back.BMP");  //背景

	//	DirectDrawCreateClipper(0, &lpDDClipper, NULL);


//Direct3DRMの初期化 ここから

		HRESULT ddret;
		GUID*   Guid;

		Direct3DRMCreate(&lpDirect3DRM);

		Guid = D3D_GuidSearch(hwnd);
		// HAL
		ddret = lpDirect3DRM->CreateDeviceFromSurface(Guid, (IDirectDraw*)lpDirectDraw, lpBackbuffer, &lpD3DRMDevice);
			strcpy(szDevice,"D3D HAL");
		if (ddret != D3DRM_OK) {
			MessageBox( hwnd, "デバイスの生成に失敗、ＨＡＬでの実行は不可能です", "", MB_OK);
			//HALでの実行が不可能な時、HELでの実行を行う
			ddret = lpDirect3DRM->CreateDeviceFromSurface(Guid, (IDirectDraw*)lpDirectDraw, lpBackbuffer, &lpD3DRMDevice);
			if (ddret != D3DRM_OK) {
				strcpy(szDevice,"HEL");
				MessageBox( hwnd, "ＨＥＬでの、デバイスの生成に失敗、Direct3Dの使用は不可能です", "", MB_OK);
			}

			if(ddret != D3DRM_OK){
				//MMX
				ddret = lpDirect3DRM->CreateDeviceFromSurface(Guid, (IDirectDraw*)lpDirectDraw, lpBackbuffer, &lpD3DRMDevice);
				strcpy(szDevice,"D3D MMX Emulation");
			}

			if(ddret != D3DRM_OK){
				//RGB
				ddret = lpDirect3DRM->CreateDeviceFromSurface(Guid, (IDirectDraw*)lpDirectDraw, lpBackbuffer, &lpD3DRMDevice);
				strcpy(szDevice,"D3D RGB Emulation");
			}
		}

		lpD3DRMDevice->SetQuality(D3DRMLIGHT_ON | D3DRMFILL_SOLID | D3DRMSHADE_GOURAUD);

		lpDirect3DRM->CreateFrame(NULL, &lpD3DRMScene);

		//カメラを作成
		lpDirect3DRM->CreateFrame(lpD3DRMScene, &lpD3DRMCamera);
		lpD3DRMCamera->SetPosition(lpD3DRMScene, D3DVAL(0.0), D3DVAL(1.0), D3DVAL(0.0));
		lpD3DRMCamera->SetOrientation(lpD3DRMScene, D3DVAL(0.0), D3DVAL(0.0), D3DVAL(1.0),  D3DVAL(0.0), D3DVAL(1.0), D3DVAL(0.0));

		//デバイスとカメラからDirect3DRMViewPortを作成
		lpDirect3DRM->CreateViewport(lpD3DRMDevice, lpD3DRMCamera, 0, 0, 640, 480, &lpD3DRMView);
		lpD3DRMView->SetBack(D3DVAL(5000.0));

//Direct3DRMの初期化 ここまで

		SetLight();

		SetObject();

		Landscape();

		while(1){

			if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
				{
					if(!GetMessage(&msg,NULL,0,0))
						return msg.wParam;
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}else{
							RenderFrame();
					}
		}
		return msg.wParam;
}
