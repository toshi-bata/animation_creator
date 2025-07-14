const animObj = JSON.parse(animDef);

// Bug fix
function reverse(value) {
    return value === 0 ? 1 : 0;
}

const homeCamera = animObj.homeCamera;
if (undefined != homeCamera) {
    homeCamera.projection = reverse(homeCamera.projection);
}

const steps = animObj.steps;
if (undefined != steps) {
    for (let step of steps) {
        if ("camera" == step.type) {
            step.camera.projection = reverse(step.camera.projection);
        }
    }
}
// Bug fis End

hwv.setCallbacks({
    modelStructureReady: () => {
        _animationSteps = new AnimationSteps(hwv);
        _animationSteps.setModel();
        
        _animationSteps.setHomeCamera(Communicator.Camera.fromJson(animObj.homeCamera));
        _animationSteps.setSteps(animObj.steps);
        
        _animationController = new AnimationController(hwv, _animationSteps);
    }
});
