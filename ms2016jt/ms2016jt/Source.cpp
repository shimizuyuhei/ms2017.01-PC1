// ms2016jt\Source.cpp
// ms2016�v���W�F�N�g�̌����e�X�g�p�v���W�F�N�g
#include <windows.h>
#include <stdio.h>
#include <locale.h>
#include <pxcsensemanager.h>
#include <pxcspeechrecognition.h>

#pragma execution_character_set("utf-8")

#pragma comment(lib, "wsock32.lib")
#define PORT 1735
#define BUFFER_SIZE 256

// �O���[�o���ϐ��̐錾
SOCKET s;
char buffer[BUFFER_SIZE];

// �v���g�^�C�v�錾
void recog_run(void);
void connect(int port, const char* ip_addr);
void close(void);

/* �����F���֘A�̃n���h�� */
class MyHandler : public PXCSpeechRecognition::Handler {
public:
	virtual void PXCAPI OnRecognition(const PXCSpeechRecognition::RecognitionData *data) {
		// �����F���̌��ʂ��󂯎�����瓮���o���n���h��
		char mess[1024];
	
		setlocale(LC_ALL, "japanese");
		wcstombs(mess, data->scores[0].sentence, 1024);

		wprintf(L"OUT: %s\n", data->scores[0].sentence);

		OutputDebugString(data->scores[0].sentence);	// <-����͏o�͂����B
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
	connect(PORT,"172.29.1.48");	// �\�P�b�g�ʐM�̏����B
	recog_run();					// �����F���̋N���B

	
	/* ��M���[�v */
	// �����Ă����R�[�h�ɂ���ĉƓd�𐧌䂷��B
	while (true) { 
		recv(s, buffer, BUFFER_SIZE, 0);
		printf("received: %s\n", buffer);
		switch (buffer[0]) {
			case 'W':
				wprintf(L"ACT:��������s���܂�");
				break;
			case 'C':
				wprintf(L"ACT:�①�ɂ����s���܂�");
				break;
			default: break;
		}
		if (GetAsyncKeyState(VK_ESCAPE)) break;
	}//looping infinitely until escape is pressed
	 //Stop the event handler that handles the speech recognition
}

// �����F���̏���
void recog_run(void) {
	// �Z�b�V�����̍쐬
	PXCSession *session = PXCSession::CreateInstance();
	if (session == NULL) {
		wprintf_s(L"Session not created by PXCSession\n");
	}

	// PXCSpeechRecognition�̓W�J
	PXCSpeechRecognition *sr = 0;
	pxcStatus sts = session->CreateImpl<PXCSpeechRecognition>(&sr);
	if (sts != pxcStatus::PXC_STATUS_NO_ERROR) {
		wprintf_s(L"Failed to create an instance of the PXCSpeechRecognition\n");
	}

	// �F���̐ݒ�(����Ȃ�)
	PXCSpeechRecognition::ProfileInfo pinfo;
	sr->QueryProfile(6, &pinfo);	// 6��n���Ɠ��{��ɐݒ肳���B
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

// �\�P�b�g�ʐM�̐ڑ�������֐�
void connect(int port, const char* ip_addr) {
	int err;
	WORD wVersionRequested = MAKEWORD(2, 2); // = 0x0202

	// �P�jWinsock���g���܂���錾
	WSADATA    wsaData;
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err == SOCKET_ERROR) {
		fprintf(stderr, "Error: Winsock Init Error\n");
	}

	// �Q�j�\�P�b�g�����܂�
	s = socket(PF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		fprintf(stderr, "Error: Socket Open Error\n");
		WSACleanup();
	}

	// �R�j�A�h���X�i�H�j�̐ݒ�
	SOCKADDR_IN    addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = inet_addr(ip_addr);

	// �S�j�f���ɐڑ�
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

// �\�P�b�g�ʐM�̐ؒf������֐�
void close(void) {
	closesocket(s);
	WSACleanup();
}