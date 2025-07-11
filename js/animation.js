class animation {
    constructor(viewer) {
        this._viewer = viewer;
        this._model = this._viewer.model;
    }

    hideAnimation(nodeIds, time) {
        var _this = this;
        var count = 0;
        var interval = _this._interval;
        var stepCount =  1;
        var stepTransparence =  1;
        
        if (interval <= time) {
            stepCount =  Math.ceil(time / interval);
            interval = time / stepCount;
            stepTransparence = 1 / time * interval;
        } else {
            _this._model.setNodesVisibility(nodeIds, false);
            return;
        }
        
        var transparence = 1;
        var id = setInterval(function () {
            transparence -= stepTransparence;
            _this._model.setNodesTransparency(nodeIds, transparence);
            count++;
            if (count >= stepCount) {
                _this._model.setNodesVisibility(nodeIds, false);
                _this._model.resetNodesTransparency(nodeIds);
                clearInterval(id);
            }
        }, interval);
    }

    translateAnimation(nodeIds, vector, time, distance, interval, assyGuideLine) {
        var _this = this;
        var count = 0;
        var stepDistance = distance / time * interval;
        var stepCount = time / interval;

        var nodeTranslationMatrixes = [];
        for (var i = 0; i < nodeIds.length; i++) {
            var nodeLocalVector = _this._convertToLocalVector(nodeIds[i], vector);
            nodeTranslationMatrixes.push(new Communicator.Matrix());
            nodeTranslationMatrixes[i].setTranslationComponent(
                nodeLocalVector.x * stepDistance, 
                nodeLocalVector.y * stepDistance, 
                nodeLocalVector.z * stepDistance);
        }

        var id = setInterval(function () {
            for (var i = 0; i < nodeIds.length; i++) {
                var nodeMatrix = _this._model.getNodeMatrix(nodeIds[i]);
                _this._model.setNodeMatrix(nodeIds[i], Communicator.Matrix.multiply(nodeMatrix, nodeTranslationMatrixes[i]));
            }
            count++;

            if (count >= stepCount) {
                clearInterval(id);
            }
        }, interval);
    }

    rotateAnimation(nodeIds, rotationAxis, basePoint, time, angle, interval, localRotation) {
        var _this = this;
        var count = 0;
        var stepAngle = angle / time * interval;
        var stepCount = time / interval;
        var translationMatrix = new Communicator.Matrix();
        var nodeRotationMatrixes = [];
        var localPoints = [];

        for (var i = 0; i < nodeIds.length; i++) {
            if (undefined == localRotation) {
                var nodeLocalAxis = _this._convertToLocalVector(nodeIds[i], rotationAxis);
                nodeRotationMatrixes.push(Communicator.Matrix.createFromOffAxisRotation(nodeLocalAxis, stepAngle));
            }
            else{
                nodeRotationMatrixes.push(new Communicator.Matrix.createFromOffAxisRotation(rotationAxis, stepAngle));
            }
            localPoints.push(_this._convertToLocalPoint(nodeIds[i], basePoint))
        }

        var id = setInterval(function () {
            count++;
            for (var i = 0; i < nodeIds.length; i++) {
                var point = Communicator.Point3.zero();
                nodeRotationMatrixes[i].transform(localPoints[i], point);
                translationMatrix.setTranslationComponent(
                    localPoints[i].x - point.x,
                    localPoints[i].y - point.y,
                    localPoints[i].z - point.z);

                var nodeMatrix = _this._model.getNodeMatrix(nodeIds[i]);
                var multiplyMatrix = Communicator.Matrix.multiply(nodeMatrix, nodeRotationMatrixes[i]);
                _this._model.setNodeMatrix(nodeIds[i], Communicator.Matrix.multiply(multiplyMatrix, translationMatrix));
            }

            if (count >= stepCount) {
                clearInterval(id);
            }
        }, interval);
    }

    _convertToLocalVector = function (nodeId, vector) {
        var _this = this;
        var parentNode = _this._model.getNodeParent(nodeId);
        var netMatrix = _this._model.getNodeNetMatrix(parentNode);
        var inverseMatrix = Communicator.Matrix.inverse(netMatrix);
        var localVector0 = Communicator.Point3.zero();
        inverseMatrix.transform(Communicator.Point3.zero(), localVector0);
        var localVector1 = Communicator.Point3.zero();
        inverseMatrix.transform(vector, localVector1);
        var localVect = new Communicator.Point3(
            localVector1.x - localVector0.x, 
            localVector1.y - localVector0.y, 
            localVector1.z - localVector0.z);

        return localVect.normalize();
    }

    _convertToLocalPoint = function (nodeId, point) {
        var _this = this;
        var parentNode = _this._model.getNodeParent(nodeId);
        var netMatrix = _this._model.getNodeNetMatrix(parentNode);
        var inverseMatrix = Communicator.Matrix.inverse(netMatrix);
        var localPoint = Communicator.Point3.zero();
        inverseMatrix.transform(point, localPoint);
        return localPoint;
    }

    blinkAnimation = function (nodeIds, time, color) {
        var _this = this;
        this._model.setNodesFaceColor(nodeIds, color);
        for (let i = 0; i < 5; i++) {
            setTimeout(() => {
                if (0 == i % 2)
                    this._model.unsetNodesFaceColor(nodeIds);
                else
                    this._model.setNodesFaceColor(nodeIds, color);
            }, time / 5 * (i + 1));
        }

    }
}
