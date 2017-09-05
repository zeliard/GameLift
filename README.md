## Test Branch for FlexMatch
[Amazon GameLift FlexMatch](http://docs.aws.amazon.com/gamelift/latest/developerguide/gamelift-match.html) is Amazon GameLift's new customizable matchmaking service. Together with GameLift Queues, FlexMatch lets you manage the full matchmaking experience in a way that best fits your game. FlexMatch manages the process from building matches and selecting players to placing matches with the best available hosting resources for an optimum player experience.

## DummyClients for FlexMatch 
Modified DummyClients project of master branch for testing FlexMatch feature.

To use FlexMatch, you need to set up a MatchMaking Rule Set. For more information, please see [here](http://docs.aws.amazon.com/gamelift/latest/developerguide/match-create-ruleset.html). See [this](https://github.com/zeliard/GameLift/blob/FlexMatch/DummyClients/match_ruleset.json) for the RuleSet file used in this project.
Next, you need to set up MatchMakingConfigurations. At this stage you need to specify the RuleSet and the Placement Queue.
Then, edit config.ini file. set GAMELIFT_REGION and MATCHMAKING_CONFIG_NAME. Build DummyClients and run.
