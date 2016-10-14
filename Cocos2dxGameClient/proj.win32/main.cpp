#include "main.h"
#include "AppDelegate.h"
#include "cocos2d.h"

#include <aws/core/Aws.h>
#include "GameLiftManager.h"

#define ALIAS_ID "Input Your ALIAS_ID"
#define GAMELIFT_REGION Aws::Region::AP_NORTHEAST_1

USING_NS_CC;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	Aws::SDKOptions options;
	Aws::InitAPI(options);

	GGameLiftManager = new GameLiftManager(ALIAS_ID);

	GGameLiftManager->SetUpAwsClient(GAMELIFT_REGION);

    // create the application instance
    AppDelegate app;

    int end = Application::getInstance()->run();

	Aws::ShutdownAPI(options);

	return end;
}
