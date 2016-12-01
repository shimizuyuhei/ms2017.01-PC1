// ms2016jt\Source.cpp
// ms2016プロジェクトの結合テスト用プロジェクト
#include <windows.h>
#include <stdio.h>
#include <locale.h>
#include <pxcsensemanager.h>
#include <pxcspeechrecognition.h>

#pragma execution_character_set("utf-8")

#pragma comment(lib, "wsock32.lib")
#define PORT 1735
#define BUFFER_SIZE 256

// グローバル変数の宣言
SOCKET s;
char buffer[BUFFER_SIZE];

// プロトタイプ宣言
void recog_run(void);
void connect(int port, const char* ip_addr);
void close(void);

/* 音声認識関連のハンドラ */
class MyHandler : public PXCSpeechRecognition::Handler {
public:
	virtual void PXCAPI OnRecognition(const PXCSpeechRecognition::RecognitionData *data) {
		// 音声認識の結果を受け取ったら動き出すハンドラ
		char mess[1024];
	
		setlocale(LC_ALL, "japanese");
		wcstombs(mess, data->scores[0].sentence, 1024);

		wprintf(L"OUT: %s\n", data->scores[0].sentence);

		OutputDebugString(data->scores[0].sentence);	// <-これは出力される。
		OutputDebugString(L"\n");

		send(s, mess, strlen(mess), 0);
		send(s, "\n", strlen("\n"), 0);
	}

	virtual void PXCAPI OnAlert(const PXCSpeechRecognition::AlertData *data) {
		if (data->label == PXCSpeechRecognition::ALERT_SPEECH_BEGIN)
			wprintf_s(L"Alert: SPEECH_BEGIN\n");
		else if (data->label == PXCSpeechRecognition::ALERT_SPEECH_END)
			wprintf_s(L"Alert: SPEECH_END\n");
	}
};

MyHandler handler;

int wmain(int argc, WCHAR* argv[]) {
	connect(PORT,"172.29.1.48");	// ソケット通信の準備。
	recog_run();					// 音声認識の起動。

	
	/* 受信ループ */
	// 送られてきたコードによって家電を制御する。
	while (true) { 
		recv(s, buffer, BUFFER_SIZE, 0);
		printf("received: %s\n", buffer);
		switch (buffer[0]) {
			case 'W':
				wprintf(L"ACT:洗濯を実行します");
				break;
			case 'C':
				wprintf(L"ACT:冷蔵庫を実行します");
				break;
			default: break;
		}
		if (GetAsyncKeyState(VK_ESCAPE)) break;
	}//looping infinitely until escape is pressed
	 //Stop the event handler that handles the speech recognition
}

// 音声認識の準備
void recog_run(void) {
	// セッションの作成
	PXCSession *session = PXCSession::CreateInstance();
	if (session == NULL) {
		wprintf_s(L"Session not created by PXCSession\n");
	}

	// PXCSpeechRecognitionの展開
	PXCSpeechRecognition *sr = 0;
	pxcStatus sts = session->CreateImpl<PXCSpeechRecognition>(&sr);
	if (sts != pxcStatus::PXC_STATUS_NO_ERROR) {
		wprintf_s(L"Failed to create an instance of the PXCSpeechRecognition\n");
	}

	// 認識の設定(言語など)
	PXCSpeechRecognition::ProfileInfo pinfo;
	sr->QueryProfile(6, &pinfo);	// 6を渡すと日本語に設定される。
	sts = sr->SetProfile(&pinfo);
	if (sts < PXC_STATUS_NO_ERROR)
	{
		wprintf_s(L"Failed to Configure the Module\n");
	}


	//Set the Recognition mode
	//command and control mode or dictation mode
	sts = sr->SetDictation();
	if (sts != pxcStatus::PXC_STATUS_NO_ERROR) {
		wprintf_s(L"Failed to Set the Recognition mode \n");
	}

	//Start the speech recognition with the event handler
	sts = sr->StartRec(NULL, &handler);
	if (sts != pxcStatus::PXC_STATUS_NO_ERROR) {
		wprintf_s(L"Failed to Start the handler \n");
	}

	if (sts != pxcStatus::PXC_STATUS_NO_ERROR) {
		wprintf_s(L"Failed to Stop the handler \n");
	}

}

// ソケット通信の接続をする関数
void connect(int port, const char* ip_addr) {
	int err;
	WORD wVersionRequested = MAKEWORD(2, 2); // = 0x0202

	// １）Winsockを使いますよ宣言
	WSADATA    wsaData;
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err == SOCKET_ERROR) {
		fprintf(stderr, "Error: Winsock Init Error\n");
	}

	// ２）ソケットを作ります
	s = socket(PF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		fprintf(stderr, "Error: Socket Open Error\n");
		WSACleanup();
	}

	// ３）アドレス（？）の設定
	SOCKADDR_IN    addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(ip_addr);

	// ４）伺かに接続
	err = connect(s, (LPSOCKADDR)&addr, sizeof(addr));
	if (err != 0) {
		fprintf(stderr, "Error: Connect Error\n");
		closesocket(s);
		WSACleanup();
	}
	else {
		fprintf(stderr, "Connect OK\n");
	}
}

// ソケット通信の切断をする関数
void close(void) {
	closesocket(s);
	WSACleanup();
}