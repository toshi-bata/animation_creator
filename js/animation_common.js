function LoadStepData(modelName) {
    return new Promise(function (resolve, reject) {
        var fileName = "jsons/" + modelName + ".json?" + (new Date).getTime();
        $.get(fileName).done(function(data, textStatus, jqXHR){
            $.getJSON(fileName, function (data) {
                if (data != undefined) {
                    resolve(data);
                } else {
                    reject("file invalid");
                }
            });
        }).fail(function() {
            reject("file not found");
        });
    });
}

class AnimationController {
    constructor(viewer, animationSteps) {
        this._viewer = viewer;
        this._animationSteps = animationSteps;
        this._homeCamera;
        this._pause = false;
        this._assyGuideLines = new Array(0);
        this._keyFrameAnimation;
        this._animation = new animation(this._viewer);
    }

    async _createKeyframeAnimation() {
        this._keyFrameAnimation = new KeyframeAnimation(this._viewer, "animation_creator");

        const homeCameraObj = this._animationSteps.getHomeCamera();
        const homeCamera = Communicator.Camera.fromJson(homeCameraObj)
        this._keyFrameAnimation.setHomeCamera(homeCamera);

        var steps = this._animationSteps.getSteps();
        var copySteps = steps.concat();
        
        copySteps.sort((a,b) => {
            if(a.startTime < b.startTime) return -1;
            if(a.startTime > b.startTime) return 1;
            return 0;
        });
        

        for (const step of copySteps) {
            const startTime = step.startTime / 1000;
            const duration = step.duration / 1000;
            const type = step.type;
            switch (type) {
                case "camera":
                    const camera = Communicator.Camera.fromJson(step.camera);
                    this._keyFrameAnimation.addCameraAnimation(camera, startTime, duration);
                    break;
                case "translation":
                    this._keyFrameAnimation.addTranslationAnimation(step.nodes, startTime, duration, step.vector, step.distance);
                    break;
                case "rotation":
                    this._keyFrameAnimation.addRotationAnimation(step.nodes, startTime, duration, step.axsis, step.center, step.angle);
                    break;
                case "hideNodes":
                    // await this._keyFrameAnimation.addOpacityAnimation(step.nodes, startTime, duration, 0);
                    await this._keyFrameAnimation.addFadeoutAnimation(step.nodes, startTime, duration, false);
                    break;
                case "blinkNodes":
                    await this._keyFrameAnimation.addBlinkAnimation(step.nodes, startTime, duration, new Communicator.Color(255, 0, 0));
                    break;
                default:
                    break;
            }
        }
    }

    async play() {
        this.rewind();
        
        // await this._createKeyframeAnimation();
        // this._keyFrameAnimation.play();

        var steps = this._animationSteps.getSteps();
        var copySteps = steps.concat();
        
        copySteps.sort((a,b) => {
            if(a.startTime < b.startTime) return -1;
            if(a.startTime > b.startTime) return 1;
            return 0;
        });

        const interval = 100;
        for (var i in copySteps) {
            var j = 0;
            setTimeout(() => {
                if (this._pause)
                    return;
                var step = copySteps[j];
                var type = step.type;
                switch(type) {
                    case "translation":
                        this._animation.translateAnimation(step.nodes, step.vector, step.duration, step.distance, interval);
                        break;
                    case "rotation":
                        this._animation.rotateAnimation(step.nodes, step.axsis, step.center, step.duration, step.angle, interval);
                        break;
                    case "camera":
                        var camera = Communicator.Camera.fromJson(step.camera);
                        this._viewer.view.setCamera(camera, step.duration);
                        break;
                    case "hideNodes":
                        this._animation.hideAnimation(step.nodes, step.duration, interval);
                        break;
                    case "blinkNodes":
                        this._animation.blinkAnimation(step.nodes, step.duration, new Communicator.Color(255, 0, 0));
                        break;
                }
                j++;
            }, copySteps[i].startTime);
        }
    }
    
    pause() {
        // this._keyFrameAnimation.pause();
        this._pause = !this._pause;
    }
    
    rewind() {
        this._viewer.model.reset();
        this._viewer.model.resetModelOpacity();

        if (undefined != this._animationSteps) {
            var cameraJson = this._animationSteps.getHomeCamera();
            if (cameraJson != undefined) {
                var camera = Communicator.Camera.fromJson(cameraJson);
                this._viewer.view.setCamera(camera);
            } else {
                this._viewer.view.resetCamera(0);
            }
        }
        else if (undefined != this._homeCamera) {
            this._viewer.view.setCamera(this._homeCamera);
        }
        else {
            this._viewer.view.resetCamera(0);
        }

        // Remove guide line
        let promiseArr = new Array(0);
        for (let assyGuideLine of this._assyGuideLines) {
            promiseArr.push(assyGuideLine.removeGuidePolylines());
        }

        if (promiseArr.length) {
            Promise.all(promiseArr).then(()=> {
                this._assyGuideLines.length = 0;
            });
        }
    }

    async serialize() {
        this.rewind();
        
        // await this._createKeyframeAnimation();
        // const obj = this._keyFrameAnimation.serialize();
        // return obj;

        return this._animationSteps.serialize();
    }

    playByObj(obj) {
        if (undefined != obj.homeCamera) {
            this._homeCamera = Communicator.Camera.fromJson(obj.homeCamera);
        }

        const animation = obj.animation;

        this._keyFrameAnimation = new KeyframeAnimation(this._viewer);
        this._keyFrameAnimation.deserialize(animation);

        this._keyFrameAnimation.play();
    }
}