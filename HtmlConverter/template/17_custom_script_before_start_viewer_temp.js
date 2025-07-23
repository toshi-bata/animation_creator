const animObj = JSON.parse(animDef);

// Because the Communicator.Projection key varies across versions, the projection object needs to be restored to its corresponding key value
animObj.homeCamera.projection = Number(Object.keys(Communicator.Projection).find((key) => Communicator.Projection[key] === animObj.homeCamera.projection));

hwv.setCallbacks({
    modelStructureReady: () => {
        _animationSteps = new AnimationSteps(hwv);
        _animationSteps.setModel();
        
        _animationSteps.setHomeCamera(Communicator.Camera.fromJson(animObj.homeCamera));
        _animationSteps.setSteps(animObj.steps);
        
        _animationController = new AnimationController(hwv, _animationSteps);
    }
});
