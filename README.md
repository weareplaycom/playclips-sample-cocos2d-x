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
