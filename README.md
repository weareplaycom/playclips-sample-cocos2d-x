# playclips-sample-cocos2d-x
Sample application implemented using Cocos2d-x Framework.

**WORK IN PROGRESS**

### Set up development environment
This repository does not hold the cocos2d framework, which is linked using a
[git submodule](https://git-scm.com/docs/git-submodule) to prevent a
considerable growth in the repository size. For that reason, while cloning the
repository you need to include the `--recursive` flag (to initialize the
submodule while cloning the repo).

```
git clone --recursive git@github.com:weareplaycom/playclips-sample-cocos2d-x.git
```

After cloning the repository, you need to download cocos2d-x dependencies using
the following script provided by cocos2d-x framework (NOTE: it requires
python2.x):

```
python cocos2d/download-deps.py
```

#### Configuration
The application can be configured in the header file `PlayClipsSampleConfig.h`.

* *AdjustEnvironment*: either sandbox (AdjustEnvironmentSandbox2dx) or production
  environment (AdjustEnvironmentProduction2dx).
* *AdjustAppToken*: a valid token, as defined in [your Adjust
  dashboard](https://www.adjust.com/). You can use [keyword
  expansion](https://git-scm.com/book/en/v2/Customizing-Git-Git-Attributes) to
  prevent having local changes in your folder. Being YOUR-REAL-TOKEN the token
  you have obtained from Adjust console, execute the following:

  ```
  git config --global filter.adjustAppToken.smudge 'sed "s/{your-app-token}/YOUR-REAL-TOKEN/"'
  git config --global filter.adjustAppToken.clean 'sed "s/YOUR-REAL-TOKEN/{your-app-token}/"'
  ```

* *AdjustLogLevel*: configure one of [the values listed in Adjust
  documentation](https://github.com/adjust/cocos2dx_sdk/blob/master/doc/ios/ios.md#sdk-logging)
