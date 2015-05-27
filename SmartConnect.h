
/**
 * Library to connect on SMARTAPPS Plataform
 * @author José Wilker <jose.wilker@smartapps.com.br>
 */


class SmartConnect {

	public:

		void begin(int sp);
		void encrypt(char login[], char pass[]);

		bool connect(char hostname[], char login[], char pass[], int port = 80);
		bool connectOnHostname();
		bool getStatusLogged();

		String envelopeRequest(char* rtype,char* rpath,char* app,char* schema,char* path,char* stringData,char* AuthID,char* PHPSESSID);

	private:

		// functions
		void readEnvelopeResponse(bool d, bool excludeHeaders);

		void getAndSetSessionId();
		void doHandShake();
		void doFlushData();

		bool readEnvelopeConnect();
		bool checkIfStatusIsSuccess();
		bool checkIfResponseIs200();

		bool parseBasedPattern(char * wData, char * pattern, int patternSize, char * closeBracket);

		// vars
		char wBuffer[1280];

		char AuthID[60];
		char PHPSESSID[26];
		char PHPSESSIDN[10];

		char* wLogin;
        char* wPass;
        char* wHostname;
        char* wRecordSet;

        String wSessionId;
        String wXsessionId;
        String wXsessionName;

        int wPort;
        int wConnectionAvaiable;
        int wSerialPort;

        // lib vars
        EthernetClient _client;

};

