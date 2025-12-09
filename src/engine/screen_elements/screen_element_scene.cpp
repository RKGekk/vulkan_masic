#include "screen_element_scene.h"

ScreenElementScene::ScreenElementScene() : Scene() {
    
};

ScreenElementScene::~ScreenElementScene() {};

bool ScreenElementScene::VOnRestore() {
    return true;
};

bool ScreenElementScene::VOnLostDevice() {
    return true;
};

void ScreenElementScene::VOnUpdate(const GameTimerDelta& delta) {};

bool ScreenElementScene::VOnRender(const GameTimerDelta& delta) {
    return true;
};
	
int ScreenElementScene::VGetZOrder() const {
    return 1;
};

void ScreenElementScene::VSetZOrder(int const zOrder) {};

bool ScreenElementScene::VIsVisible() const {
    return true;
};

void ScreenElementScene::VSetVisible(bool visible) {

};

bool ScreenElementScene::VAddChild(std::shared_ptr<SceneNode> kid) {
    return true;
};

void ScreenElementScene::ModifiedSceneNodeComponentDelegate(IEventDataPtr pEventData) {};

void ScreenElementScene::ModifiedSceneNode(std::shared_ptr<SceneNode> node) {};

void ScreenElementScene::RegisterAllDelegates() {};

void ScreenElementScene::RemoveAllDelegates() {};