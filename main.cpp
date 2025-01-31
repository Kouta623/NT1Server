#include<Novice.h>
#include <math.h>
#include <process.h>
#include <mmsystem.h>

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "winmm.lib")

DWORD WINAPI Threadfunc(void*);
HWND hwMain;
const char kWindowTitle[] = "KAMATA ENGINEサーバ";

typedef struct {
	float x;
	float y;
}Vector2;

typedef struct {
	Vector2 center;
	float radius;
}Circle;

Circle a, b;
Vector2 center = { 100,100 };
char keys[256] = { 0 };
char preKeys[256] = { 0 };
int color = RED;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WSADATA wdData;
	static HANDLE hThread;
	static DWORD dwID;


	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	hwMain = GetDesktopWindow();

	a.center.x = 400;
	a.center.y = 400;
	a.radius = 100;

	b.center.x = 200;
	b.center.y = 200;
	b.radius = 50;

	// winsock初期化
	WSAStartup(MAKEWORD(2, 0), &wdData);

	// ソケット通信用スレッド作成
	hThread = (HANDLE)CreateThread(NULL, 0, &Threadfunc, (LPVOID)&a, 0, &dwID);

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレーム開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		if (keys[DIK_UP] != 0) {
			b.center.y -= 15;
		}
		if (keys[DIK_DOWN] != 0) {
			b.center.y += 15;
		}
		if (keys[DIK_RIGHT] != 0) {
			b.center.x += 15;
		}
		if (keys[DIK_LEFT] != 0) {
			b.center.x -= 15;
		}
		//　☆上下左右キー入力されてたら円bの座標を更新（加算値は常識的な値で）

		// ↓更新処理ここから
		float distance =
			sqrtf((float)pow((double)a.center.x - (double)b.center.x, 2) +
				(float)pow((double)a.center.y - (double)b.center.y, 2));

		

		if (distance <= a.radius + b.radius) {
			color = BLUE;
		}
		else color = RED;

		
		// ↑更新処理ここまで

		// ↓描画処理ここから
		Novice::DrawEllipse((int)a.center.x, (int)a.center.y, (int)a.radius, (int)a.radius, 0.0f, WHITE, kFillModeSolid);
		Novice::DrawEllipse((int)b.center.x, (int)b.center.y, (int)b.radius, (int)b.radius, 0.0f, color, kFillModeSolid);
		if (color == BLUE) {
			Novice::DrawBox(0, 0, 1280, 720, 0.0f, GREEN, kFillModeSolid);
		}
		// ↑描画処理ここまで

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリ終了
	Novice::Finalize();

	// winsock終了
	WSACleanup();

	return 0;
}

/* 通信スレッド関数 */
DWORD WINAPI Threadfunc(void * ) {

	SOCKET sWait, sConnect;
	WORD wPort = 8000;
	SOCKADDR_IN saConnect, saLocal;
	int iLen, iRecv;

	// リスンソケット
	sWait = socket(PF_INET, SOCK_STREAM, 0);

	ZeroMemory(&saLocal, sizeof(saLocal));

	// 8000番に接続待機用ソケット作成
	saLocal.sin_family = AF_INET;
	saLocal.sin_addr.s_addr = INADDR_ANY;
	saLocal.sin_port = htons(wPort);

	if (bind(sWait, (LPSOCKADDR)&saLocal, sizeof(saLocal)) == SOCKET_ERROR) {

		closesocket(sWait);
		SetWindowText(hwMain, L"接続待機ソケット失敗");
		return 1;
	}

	if (listen(sWait, 2) == SOCKET_ERROR) {

		closesocket(sWait);
		SetWindowText(hwMain, L"接続待機ソケット失敗");
		return 1;
	}

	SetWindowText(hwMain, L"接続待機ソケット成功");

	iLen = sizeof(saConnect);

	// sConnectに接続受け入れ
	sConnect = accept(sWait, (LPSOCKADDR)(&saConnect), &iLen);

	// 接続待ち用ソケット解放
	closesocket(sWait);

	if (sConnect == INVALID_SOCKET) {

		shutdown(sConnect, 2);
		closesocket(sConnect);
		shutdown(sWait, 2);
		closesocket(sWait);

		SetWindowText(hwMain, L"ソケット接続失敗");

		return 1;
	}

	SetWindowText(hwMain, L"ソケット接続成功");

	iRecv = 0;

	while (1)
	{
		int     nRcv;

		// データ受け取り
		nRcv = recv(sConnect, (char*)&a, sizeof(a), 0);

		if (nRcv == SOCKET_ERROR)break;

		// メッセージ送信
		send(sConnect, (const char*)&b, sizeof(b), 0);

	}

	shutdown(sConnect, 2);
	closesocket(sConnect);

	return 0;
}