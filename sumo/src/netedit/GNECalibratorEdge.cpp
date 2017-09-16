/****************************************************************************/
/// @file    GNECalibratorEdge.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Sep 2017
/// @version $Id: GNECalibratorEdge.cpp 25918 2017-09-07 19:38:16Z behrisch $
///
//
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.dlr.de/
// Copyright (C) 2001-2017 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This file is part of SUMO.
//   SUMO is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/

// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <iostream>
#include <utility>
#include <foreign/polyfonts/polyfonts.h>
#include <utils/geom/PositionVector.h>
#include <utils/common/RandHelper.h>
#include <utils/common/SUMOVehicleClass.h>
#include <utils/common/ToString.h>
#include <utils/geom/GeomHelper.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/images/GUIIconSubSys.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>
#include <utils/gui/div/GUIGlobalSelection.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/images/GUITexturesHelper.h>
#include <utils/xml/SUMOSAXHandler.h>

#include "GNECalibratorEdge.h"
#include "GNEEdge.h"
#include "GNELane.h"
#include "GNEViewNet.h"
#include "GNEUndoList.h"
#include "GNENet.h"
#include "GNEChange_Attribute.h"
#include "GNERouteProbe.h"
#include "GNECalibratorDialog.h"


// ===========================================================================
// member method definitions
// ===========================================================================

GNECalibratorEdge::GNECalibratorEdge(const std::string& id, GNEEdge* edge, GNEViewNet* viewNet, double pos,
                             double frequency, const std::string& output, const std::vector<GNECalibratorRoute>& calibratorRoutes,
                             const std::vector<GNECalibratorFlow>& calibratorFlows, const std::vector<GNECalibratorVehicleType>& calibratorVehicleTypes) :
    GNECalibrator(id, viewNet, pos, frequency, output, calibratorRoutes, calibratorFlows, calibratorVehicleTypes)  {
    // This additional belong to an edge
    myEdge = edge;
    // this additional ISN'T movable
    myMovable = false;
    // Update geometry;
    updateGeometry();
    // Center view in the position of calibrator
    myViewNet->centerTo(getGlID(), false);
}


GNECalibratorEdge::~GNECalibratorEdge() {}


void
GNECalibratorEdge::writeAdditional(OutputDevice& device, bool volatileOptionsEnabled) const {
    // Write parameters
    device.openTag(getTag());
    device.writeAttr(SUMO_ATTR_ID, getID());
    device.writeAttr(SUMO_ATTR_EDGE, myEdge->getID());
    device.writeAttr(SUMO_ATTR_POSITION, myPositionOverLane * myEdge->getLanes().at(0)->getLaneParametricLength());
    device.writeAttr(SUMO_ATTR_FREQUENCY, myFrequency);
    device.writeAttr(SUMO_ATTR_OUTPUT, myOutput);
    // write all routes of this calibrator
    for (std::vector<GNECalibratorRoute>::const_iterator i = myCalibratorRoutes.begin(); i != myCalibratorRoutes.end(); ++i) {
        // Open route tag
        device.openTag(i->getTag());
        // Write route ID
        device.writeAttr(SUMO_ATTR_BEGIN, i->getRouteID());
        // Write edge IDs
        device.writeAttr(SUMO_ATTR_BEGIN, i->getEdgesIDs());
        // Write Color
        device.writeAttr(SUMO_ATTR_BEGIN, i->getColor());
        // Close flow tag
        device.closeTag();
    }
    // write all vehicle types of this calibrator
    for (std::vector<GNECalibratorVehicleType>::const_iterator i = myCalibratorVehicleTypes.begin(); i != myCalibratorVehicleTypes.end(); ++i) {
        // Open vehicle type tag
        device.openTag(i->getTag());
        // write id
        device.writeAttr(SUMO_ATTR_ID, i->getVehicleTypeID());
        //write accel
        device.writeAttr(SUMO_ATTR_ACCEL, i->getAccel());
        // write decel
        device.writeAttr(SUMO_ATTR_DECEL, i->getDecel());
        // write sigma
        device.writeAttr(SUMO_ATTR_SIGMA, i->getSigma());
        // write tau
        device.writeAttr(SUMO_ATTR_TAU, i->getTau());
        // write lenght
        device.writeAttr(SUMO_ATTR_LENGTH, i->getLength());
        // write min gap
        device.writeAttr(SUMO_ATTR_MINGAP, i->getMinGap());
        // write max speed
        device.writeAttr(SUMO_ATTR_MAXSPEED, i->getMaxSpeed());
        // write speed factor
        device.writeAttr(SUMO_ATTR_SPEEDFACTOR, i->getSpeedFactor());
        // write speed dev
        device.writeAttr(SUMO_ATTR_SPEEDDEV, i->getSpeedDev());
        // write color
        device.writeAttr(SUMO_ATTR_COLOR, i->getColor());
        // write vehicle class
        device.writeAttr(SUMO_ATTR_VCLASS, i->getVClass());
        // write emission class
        device.writeAttr(SUMO_ATTR_EMISSIONCLASS, i->getEmissionClass());
        // write shape
        device.writeAttr(SUMO_ATTR_SHAPE, i->getShape());
        // write width
        device.writeAttr(SUMO_ATTR_WIDTH, i->getWidth());
        // write filename
        device.writeAttr(SUMO_ATTR_FILE, i->getFilename());
        // write impatience
        device.writeAttr(SUMO_ATTR_IMPATIENCE, i->getImpatience());
        // write lane change model
        device.writeAttr(SUMO_ATTR_LANE_CHANGE_MODEL, i->getLaneChangeModel());
        // write car follow model
        device.writeAttr(SUMO_ATTR_CAR_FOLLOW_MODEL, i->getCarFollowModel());
        // write person capacity
        device.writeAttr(SUMO_ATTR_PERSON_CAPACITY, i->getPersonCapacity());
        // write container capacity
        device.writeAttr(SUMO_ATTR_CONTAINER_CAPACITY, i->getContainerCapacity());
        // write boarding duration
        device.writeAttr(SUMO_ATTR_BOARDING_DURATION, i->getBoardingDuration());
        // write loading duration
        device.writeAttr(SUMO_ATTR_LOADING_DURATION, i->getLoadingDuration());
        // write get lat alignment
        device.writeAttr(SUMO_ATTR_LATALIGNMENT, i->getLatAlignment());
        // write min gap lat
        device.writeAttr(SUMO_ATTR_MINGAP_LAT, i->getMinGapLat());
        // write max speed lat
        device.writeAttr(SUMO_ATTR_MAXSPEED_LAT, i->getMaxSpeedLat());
        // Close vehicle type tag
        device.closeTag();
    }
    // Write all flows of this calibrator
    for (std::vector<GNECalibratorFlow>::const_iterator i = myCalibratorFlows.begin(); i != myCalibratorFlows.end(); ++i) {
        // Open flow tag
        device.openTag(i->getTag());
        // Write begin
        device.writeAttr(SUMO_ATTR_BEGIN, i->getBegin());
        // Write end
        device.writeAttr(SUMO_ATTR_END, i->getEnd());
        // Write type
        device.writeAttr(SUMO_ATTR_TYPE, i->getVehicleType());
        // Write route
        device.writeAttr(SUMO_ATTR_ROUTE, i->getRoute());
        // Write color
        device.writeAttr(SUMO_ATTR_COLOR, i->getColor());
        // Write depart lane
        device.writeAttr(SUMO_ATTR_DEPARTLANE, i->getDepartLane());
        // Write depart pos
        device.writeAttr(SUMO_ATTR_DEPARTPOS, i->getDepartPos());
        // Write depart speed
        device.writeAttr(SUMO_ATTR_DEPARTSPEED, i->getDepartSpeed());
        // Write arrival lane
        device.writeAttr(SUMO_ATTR_ARRIVALLANE, i->getArrivalLane());
        // Write arrival pos
        device.writeAttr(SUMO_ATTR_ARRIVALPOS, i->getArrivalPos());
        // Write arrival speed
        device.writeAttr(SUMO_ATTR_ARRIVALSPEED, i->getArrivalSpeed());
        // Write line
        device.writeAttr(SUMO_ATTR_LINE, i->getLine());
        // Write person number
        device.writeAttr(SUMO_ATTR_PERSON_NUMBER, i->getPersonNumber());
        // Write container number
        device.writeAttr(SUMO_ATTR_CONTAINER_NUMBER, i->getContainerNumber());
        // Write reroute
        device.writeAttr(SUMO_ATTR_REROUTE, i->getReroute());
        // Write departPosLat
        device.writeAttr(SUMO_ATTR_DEPARTPOS_LAT, i->getDepartPosLat());
        // Write arrivalPosLat
        device.writeAttr(SUMO_ATTR_ARRIVALPOS_LAT, i->getArrivalPosLat());
        // Write number
        device.writeAttr(SUMO_ATTR_NUMBER, i->getNumber());
        // Write type of flow
        if (i->getFlowType() == GNECalibratorFlow::GNE_CALIBRATORFLOW_PERIOD) {
            // write period
            device.writeAttr(SUMO_ATTR_PERIOD, i->getPeriod());
        } else if (i->getFlowType() == GNECalibratorFlow::GNE_CALIBRATORFLOW_VEHSPERHOUR) {
            // write vehs per hour
            device.writeAttr(SUMO_ATTR_VEHSPERHOUR, i->getVehsPerHour());
        } else if (i->getFlowType() == GNECalibratorFlow::GNE_CALIBRATORFLOW_PROBABILITY) {
            // write probability
            device.writeAttr(SUMO_ATTR_PROB, i->getProbability());
        }
        // Close flow tag
        device.closeTag();
    }
    // Close tag
    device.closeTag();
}


void
GNECalibratorEdge::updateGeometry() {
    // Clear all containers
    myShapeRotations.clear();
    myShapeLengths.clear();
    // clear Shape
    myShape.clear();

    // Get shape of lane parent
    myShape.push_back(myEdge->getLanes().at(0)->getShape().positionAtOffset(myPositionOverLane * myEdge->getLanes().at(0)->getShape().length()));

    // Obtain first position
    Position f = myShape[0] - Position(1, 0);

    // Obtain next position
    Position s = myShape[0] + Position(1, 0);

    // Save rotation (angle) of the vector constructed by points f and s
    myShapeRotations.push_back(myEdge->getLanes().at(0)->getShape().rotationDegreeAtOffset(myPositionOverLane * myEdge->getLanes().at(0)->getShape().length()) * -1);

    // Refresh element (neccesary to avoid grabbing problems)
    myViewNet->getNet()->refreshAdditional(this);
}


void
GNECalibratorEdge::drawGL(const GUIVisualizationSettings& s) const {
    // get values
    glPushName(getGlID());
    glLineWidth(1.0);
    const double exaggeration = s.addSize.getExaggeration(s);

    for (int i = 0; i < (int)myShape.size(); ++i) {
        const Position& pos = myShape[i];
        double rot = myShapeRotations[i];
        glPushMatrix();
        glTranslated(pos.x(), pos.y(), getType());
        glRotated(rot, 0, 0, 1);
        glTranslated(0, 0, getType());
        glScaled(exaggeration, exaggeration, 1);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if (isAdditionalSelected()) {
            GLHelper::setColor(myViewNet->getNet()->selectedAdditionalColor);
        } else {
            GLHelper::setColor(RGBColor(255, 204, 0));
        }
        // base
        glBegin(GL_TRIANGLES);
        glVertex2d(0 - 1.4, 0);
        glVertex2d(0 - 1.4, 6);
        glVertex2d(0 + 1.4, 6);
        glVertex2d(0 + 1.4, 0);
        glVertex2d(0 - 1.4, 0);
        glVertex2d(0 + 1.4, 6);
        glEnd();

        // draw text
        if (s.scale * exaggeration >= 1.) {
            GLHelper::setColor(RGBColor(0, 0, 0));
            glTranslated(0, 0, .1);
            pfSetPosition(0, 0);
            pfSetScale(3.f);
            double w = pfdkGetStringWidth("C");
            glRotated(180, 0, 1, 0);
            glTranslated(-w / 2., 2, 0);
            pfDrawString("C");
            glTranslated(w / 2., -2, 0);
        }
        glPopMatrix();
    }
    drawName(getCenteringBoundary().getCenter(), s.scale, s.addName);
    glPopName();
}


std::string
GNECalibratorEdge::getAttribute(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_ID:
            return getAdditionalID();
        case SUMO_ATTR_EDGE:
            return toString(myEdge->getID());
        case SUMO_ATTR_POSITION:
            return toString(myPositionOverLane * myEdge->getLanes().at(0)->getLaneParametricLength());
        case SUMO_ATTR_FREQUENCY:
            return toString(myFrequency);
        case SUMO_ATTR_OUTPUT:
            return myOutput;
        case SUMO_ATTR_ROUTEPROBE:
            if (myRouteProbe) {
                return myRouteProbe->getID();
            } else {
                return "";
            }
        default:
            throw InvalidArgument(toString(getTag()) + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNECalibratorEdge::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_EDGE:
        case SUMO_ATTR_POSITION:
        case SUMO_ATTR_FREQUENCY:
        case SUMO_ATTR_OUTPUT:
        case SUMO_ATTR_ROUTEPROBE:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            updateGeometry();
            break;
        default:
            throw InvalidArgument(toString(getTag()) + " doesn't have an attribute of type '" + toString(key) + "'");
    }

}


bool
GNECalibratorEdge::isValid(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            if (isValidID(value) && (myViewNet->getNet()->getAdditional(getTag(), value) == NULL)) {
                return true;
            } else {
                return false;
            }
        case SUMO_ATTR_EDGE:
            if (myViewNet->getNet()->retrieveEdge(value, false) != NULL) {
                return true;
            } else {
                return false;
            }
        case SUMO_ATTR_POSITION:
            if (canParse<double>(value)) {
                // obtain relative new start position
                double newStartPos = parse<double>(value) / myEdge->getLanes().at(0)->getLaneParametricLength();
                if ((newStartPos < 0) || (newStartPos > 1)) {
                    return false;
                } else {
                    return true;
                }
            }
        case SUMO_ATTR_FREQUENCY:
            return (canParse<double>(value) && parse<double>(value) >= 0);
        case SUMO_ATTR_OUTPUT:
            return isValidFilename(value);
        case SUMO_ATTR_ROUTEPROBE:
            if (myViewNet->getNet()->getAdditional(SUMO_TAG_ROUTEPROBE, value) != NULL) {
                return true;
            } else {
                return false;
            }
        default:
            throw InvalidArgument(toString(getTag()) + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}

// ===========================================================================
// private
// ===========================================================================

void
GNECalibratorEdge::setAttribute(SumoXMLAttr key, const std::string& value) {
    switch (key) {
        case SUMO_ATTR_ID:
            setAdditionalID(value);
            break;
        case SUMO_ATTR_EDGE:
            changeEdge(value);
            break;
        case SUMO_ATTR_POSITION:
            myPositionOverLane = parse<double>(value) / myEdge->getLanes().at(0)->getShape().length();
            updateGeometry();
            getViewNet()->update();
            break;
        case SUMO_ATTR_FREQUENCY:
            myFrequency = parse<double>(value);
            break;
        case SUMO_ATTR_OUTPUT:
            myOutput = value;
            break;
        case SUMO_ATTR_ROUTEPROBE:
            myRouteProbe = dynamic_cast<GNERouteProbe*>(myViewNet->getNet()->getAdditional(SUMO_TAG_ROUTEPROBE, value));
            break;
        default:
            throw InvalidArgument(toString(getTag()) + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}

/****************************************************************************/