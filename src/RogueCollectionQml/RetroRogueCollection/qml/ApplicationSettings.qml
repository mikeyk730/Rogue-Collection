/*******************************************************************************
* Copyright (c) 2013 "Filippo Scognamiglio"
* https://github.com/Swordfish90/cool-retro-term
*
* This file is part of cool-retro-term.
*
* cool-retro-term is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

import QtQuick 2.2
import QtQuick.Controls 1.0

import "utils.js" as Utils

QtObject{
    property string version: "3.1.0"

    property Item rogue

    // STATIC CONSTANTS ////////////////////////////////////////////////////////

    readonly property real minimumFontScaling: 0.25
    readonly property real maximumFontScaling: 2.50

    // GENERAL SETTINGS ///////////////////////////////////////////////////////

    property int x: 100
    property int y: 100
    property int width: 1024
    property int height: 768

    property bool fullscreen: false
    property bool showMenubar: true

    property real windowOpacity: 1.0
    property real ambientLight: 0.0
    property real contrast: 1.0
    property real brightness: 1.0/3.0

    property bool showTerminalSize: true
    property real windowScaling: 1.0

    property real fps: 24
    property bool verbose: false

    property int windowScale: defaultScale
    property bool maintainAspect: true
    property bool sound: false

    onWindowScalingChanged: handleFontChanged();

    // PROFILE SETTINGS ///////////////////////////////////////////////////////

    property string graphics

    property string _backgroundColor: "#000000"
    property string _fontColor: "#ffffff"
    property string saturatedColor: Utils.mix(Utils.strToColor("#FFFFFF"), Utils.strToColor(_fontColor), saturationColor * 0.5)
    property color fontColor: Utils.mix(Utils.strToColor(saturatedColor), Utils.strToColor(_backgroundColor), 0.7 + (contrast * 0.3))
    property color backgroundColor: Utils.mix(Utils.strToColor(_backgroundColor), Utils.strToColor(saturatedColor), 0.7 + (contrast * 0.3))

    property real staticNoise: 0
    property real screenCurvature: 0
    property real glowingLine: 0
    property real burnIn: 0
    property real bloom: 0

    property real bloomQuality: 0.5
    property real burnInQuality: 0.5

    property real chromaColor: 1
    property real saturationColor: 0.0

    property real jitter: 0

    property real horizontalSync: 0
    property real flickering: 0

    property real rbgShift: 0.0

    readonly property int no_rasterization: 0
    readonly property int scanline_rasterization: 1
    readonly property int pixel_rasterization: 2

    property int rasterization: no_rasterization

    // FONTS //////////////////////////////////////////////////////////////////

    property real fontScaling: 1.0
    property real fontWidth: 1.0

    property bool lowResolutionFont: false

    property var fontNames: ["IBM_DOS", "TERMINUS_SCALED"]
    property var fontlist: fontManager.item.fontlist

    signal terminalFontChanged(string fontSource, int pixelSize, int lineSpacing, real screenScaling, real fontWidth)

    signal initializedSettings()

    property Loader fontManager: Loader{
        states: [
            State { when: rasterization == no_rasterization
                PropertyChanges {target: fontManager; source: "Fonts.qml" } },
            State { when: rasterization == scanline_rasterization
                PropertyChanges {target: fontManager; source: "FontScanlines.qml" } },
            State { when: rasterization == pixel_rasterization;
                PropertyChanges {target: fontManager; source: "FontPixels.qml" } }
        ]

        onLoaded: handleFontChanged()
    }

    onFontScalingChanged: handleFontChanged();
    onFontWidthChanged: handleFontChanged();

    function getIndexByName(name) {
        for (var i = 0; i < fontlist.count; i++) {
            if (name === fontlist.get(i).name)
                return i;
        }
        return 0; // If the font is not available default to 0.
    }

    function incrementScaling(){
        fontScaling = Math.min(fontScaling + 0.05, maximumFontScaling);
        handleFontChanged();
    }

    function decrementScaling(){
        fontScaling = Math.max(fontScaling - 0.05, minimumFontScaling);
        handleFontChanged();
    }

    function handleFontChanged(){
        if (!fontManager.item) return;

        var index = getIndexByName(fontNames[rasterization]);
        if (index === undefined) return;

        fontManager.item.selectedFontIndex = index;
        fontManager.item.scaling = fontScaling * windowScaling;

        var fontSource = fontManager.item.source;
        var pixelSize = fontManager.item.pixelSize;
        var lineSpacing = fontManager.item.lineSpacing;
        var screenScaling = fontManager.item.screenScaling;
        var fontWidth = fontManager.item.defaultFontWidth * appSettings.fontWidth;

        lowResolutionFont = fontManager.item.lowResolutionFont;

        terminalFontChanged(fontSource, pixelSize, lineSpacing, screenScaling, fontWidth);
    }

    // FRAMES /////////////////////////////////////////////////////////////////

    property ListModel graphicsList: ListModel{
        ListElement {
            name: ""
            text: "Default"
        }
        ListElement {
            name: "unix"
            text: "Unix"
        }
        ListElement {
            name: "pc"
            text: "PC"
        }
        ListElement {
            name: "tiles"
            text: "Tiles"
        }
        ListElement {
            name: "boxy"
            text: "Boxy"
        }
    }

    function getGraphicsIndexByName(name) {
        for (var i = 0; i < graphicsList.count; i++) {
            if (name === graphicsList.get(i).name)
                return i;
        }
        return 0;
    }


    property ListModel framesList: ListModel{
        ListElement{
            name: "NO_FRAME"
            text: "No frame"
            source: ""
            reflections: false
        }
        ListElement{
            name: "SIMPLE_WHITE_FRAME"
            text: "Simple white frame"
            source: "./frames/WhiteSimpleFrame.qml"
            reflections: true
        }
        ListElement{
            name: "ROUGH_BLACK_FRAME"
            text: "Rough black frame"
            source: "./frames/BlackRoughFrame.qml"
            reflections: true
        }
    }

    function getFrameIndexByName(name) {
        for (var i = 0; i < framesList.count; i++) {
            if (name === framesList.get(i).name)
                return i;
        }
        return 0; // If the frame is not available default to 0.
    }

    property string frameSource: ""
    property string frameName: "NO_FRAME"

    property bool _frameReflections: false
    property bool reflectionsAllowed: true
    property bool frameReflections: _frameReflections && reflectionsAllowed

    onFrameNameChanged: {
        var index = getFrameIndexByName(frameName);
        frameSource = framesList.get(index).source;
        reflectionsAllowed = framesList.get(index).reflections;
    }

    // DB STORAGE /////////////////////////////////////////////////////////////

    property Storage storage: Storage{ }

    function stringify(obj) {
        var replacer = function(key, val) {
            return val.toFixed ? Number(val.toFixed(4)) : val;
        }
        return JSON.stringify(obj, replacer, 2);
    }

    function composeSettingsString(){
        var settings = {
            fps: fps,
            x: x,
            y: y,
            width: width,
            height: height,
            windowScaling: windowScaling,
            showTerminalSize: showTerminalSize,
            fontScaling: fontScaling,
            fontNames: fontNames,
            frameReflections: _frameReflections,
            bloomQuality: bloomQuality,
            burnInQuality: burnInQuality,
            windowScale: windowScale,
            maintainAspect: maintainAspect,
            sound: sound
        }
        return stringify(settings);
    }

    function composeProfileObject(){
        var settings = {
            backgroundColor: _backgroundColor,
            fontColor: _fontColor,
            flickering: flickering,
            horizontalSync: horizontalSync,
            staticNoise: staticNoise,
            chromaColor: chromaColor,
            saturationColor: saturationColor,
            screenCurvature: screenCurvature,
            glowingLine: glowingLine,
            frameName: frameName,
            burnIn: burnIn,
            bloom: bloom,
            rasterization: rasterization,
            jitter: jitter,
            rbgShift: rbgShift,
            brightness: brightness,
            contrast: contrast,
            ambientLight: ambientLight,
            windowOpacity: windowOpacity,
            fontName: fontNames[rasterization],
            fontWidth: fontWidth,
            graphics: graphics
        }
        return settings;
    }

    function composeProfileString() {
        return stringify(composeProfileObject());
    }

    function loadSettings(){
        var settingsString = storage.getSetting("_CURRENT_SETTINGS");
        var profileString = storage.getSetting("_CURRENT_PROFILE");

        if(!settingsString) return;
        if(!profileString) return;

        loadSettingsString(settingsString);
        loadProfileString(profileString);

        if (verbose)
            console.log("Loading settings: " + settingsString + profileString);
    }

    function storeSettings(){
        var settingsString = composeSettingsString();
        var profileString = composeProfileString();

        storage.setSetting("_CURRENT_SETTINGS", settingsString);
        storage.setSetting("_CURRENT_PROFILE", profileString);

        if (verbose) {
            console.log("Storing settings: " + settingsString);
            console.log("Storing profile: " + profileString);
        }
    }

    function loadSettingsString(settingsString){
        var settings = JSON.parse(settingsString);

        showTerminalSize = settings.showTerminalSize !== undefined ? settings.showTerminalSize : showTerminalSize

        fps = settings.fps !== undefined ? settings.fps: fps
        windowScaling = settings.windowScaling !== undefined ? settings.windowScaling : windowScaling

        x = settings.x !== undefined ? settings.x : x
        y = settings.y !== undefined ? settings.y : y
        width = settings.width !== undefined ? settings.width : width
        height = settings.height !== undefined ? settings.height : height

        fontNames = settings.fontNames !== undefined ? settings.fontNames : fontNames
        fontScaling = settings.fontScaling !== undefined ? settings.fontScaling : fontScaling

        _frameReflections = settings.frameReflections !== undefined ? settings.frameReflections : _frameReflections;

        //showMenubar = settings.showMenubar !== undefined ? settings.showMenubar : showMenubar;

        bloomQuality = settings.bloomQuality !== undefined ? settings.bloomQuality : bloomQuality;
        burnInQuality = settings.burnInQuality !== undefined ? settings.burnInQuality : burnInQuality;

        windowScale = settings.windowScale !== undefined ? settings.windowScale : windowScale;
        maintainAspect = settings.maintainAspect !== undefined ? settings.maintainAspect : maintainAspect;
        sound = settings.sound !== undefined ? settings.sound : sound;
    }

    function loadProfileString(profileString){
        var settings = JSON.parse(profileString);

        _backgroundColor = settings.backgroundColor !== undefined ? settings.backgroundColor : _backgroundColor;
        _fontColor = settings.fontColor !== undefined ? settings.fontColor : _fontColor;

        horizontalSync = settings.horizontalSync !== undefined ? settings.horizontalSync : horizontalSync
        flickering = settings.flickering !== undefined ? settings.flickering : flickering;
        staticNoise = settings.staticNoise !== undefined ? settings.staticNoise : staticNoise;
        chromaColor = settings.chromaColor !== undefined ? settings.chromaColor : chromaColor;
        saturationColor = settings.saturationColor !== undefined ? settings.saturationColor : saturationColor;
        screenCurvature = settings.screenCurvature !== undefined ? settings.screenCurvature : screenCurvature;
        glowingLine = settings.glowingLine !== undefined ? settings.glowingLine : glowingLine;

        burnIn = settings.burnIn !== undefined ? settings.burnIn : burnIn
        bloom = settings.bloom !== undefined ? settings.bloom : bloom

        frameName = settings.frameName !== undefined ? settings.frameName : frameName;

        rasterization = settings.rasterization !== undefined ? settings.rasterization : rasterization;

        jitter = settings.jitter !== undefined ? settings.jitter : jitter;

        rbgShift = settings.rbgShift !== undefined ? settings.rbgShift : rbgShift;

        ambientLight = settings.ambientLight !== undefined ? settings.ambientLight : ambientLight;
        contrast = settings.contrast !== undefined ? settings.contrast : contrast;
        brightness = settings.brightness !== undefined ? settings.brightness : brightness;
        windowOpacity = settings.windowOpacity !== undefined ? settings.windowOpacity : windowOpacity;

        fontNames[rasterization] = settings.fontName !== undefined ? settings.fontName : fontNames[rasterization];
        fontWidth = settings.fontWidth !== undefined ? settings.fontWidth : fontWidth;

        graphics = settings.graphics !== undefined ? settings.graphics : graphics

        handleFontChanged();
    }

    function storeCustomProfiles(){
        storage.setSetting("_CUSTOM_PROFILES", composeCustomProfilesString());
    }

    function loadCustomProfiles(){
        var customProfileString = storage.getSetting("_CUSTOM_PROFILES");
        if(customProfileString === undefined) customProfileString = "[]";
        loadCustomProfilesString(customProfileString);
    }

    function loadCustomProfilesString(customProfilesString){
        var customProfiles = JSON.parse(customProfilesString);
        for (var i=0; i<customProfiles.length; i++) {
            var profile = customProfiles[i];

            if (verbose)
                console.log("Loading custom profile: " + stringify(profile));

            profilesList.append(profile);
        }
    }

    function composeCustomProfilesString(){
        var customProfiles = []
        for(var i=0; i<profilesList.count; i++){
            var profile = profilesList.get(i);
            if(profile.builtin) continue;
            customProfiles.push({text: profile.text, obj_string: profile.obj_string, builtin: false})
        }
        return stringify(customProfiles);
    }

    function loadProfile(index){
        var profile = profilesList.get(index);
        loadProfileString(profile.obj_string);
    }

    function appendCustomProfile(name, profileString) {
        profilesList.append({text: name, obj_string: profileString, builtin: false});
    }

    // PROFILES ///////////////////////////////////////////////////////////////

    property ListModel profilesList: ListModel{
        ListElement{
            text: "No Effect PC"
            obj_string: '{"ambientLight":0,"backgroundColor":"#000000","bloom":0,"brightness":0.33,"flickering":0.0,"contrast":1.0,"fontName":"IBM_DOS","fontColor":"#ffffff","frameName":"NO_FRAME","glowingLine":0,"horizontalSync":0,"jitter":0,"burnIn":0,"staticNoise":0.0,"rasterization":0,"screenCurvature":0.0,"windowOpacity":1,"chromaColor":1,"saturationColor":0,"rbgShift":0.0,"fontWidth":1.0,"useCustomCommand":false,"graphics":"pc"}'
            builtin: true
        }
        ListElement{
            text: "No Effect Unix"
            obj_string: '{"ambientLight":0,"backgroundColor":"#000000","bloom":0,"brightness":0.33,"flickering":0.0,"contrast":1.0,"fontName":"TERMINUS_SCALED","fontColor":"#ffffff","frameName":"NO_FRAME","glowingLine":0,"horizontalSync":0,"jitter":0,"burnIn":0,"staticNoise":0.0,"rasterization":0,"screenCurvature":0.0,"windowOpacity":1,"chromaColor":1,"saturationColor":0,"rbgShift":0.0,"fontWidth":1.0,"useCustomCommand":false,"graphics":"unix"}'
            builtin: true
        }
        ListElement{
            text: "IBM Dos"
            obj_string: '{"ambientLight":0.07,"backgroundColor":"#000000","bloom":0.33,"brightness":0.25,"flickering":0.0,"contrast":0.95,"fontName":"IBM_DOS","fontColor":"#ffffff","frameName":"SIMPLE_WHITE_FRAME","glowingLine":0,"horizontalSync":0,"jitter":0.08,"burnIn":0.0,"staticNoise":0.0,"rasterization":0,"screenCurvature":0.07,"windowOpacity":1,"chromaColor":1,"saturationColor":0,"rbgShift":0.0,"fontWidth":1.0,"useCustomCommand":false,"graphics":"pc"}'
            builtin: true
        }
        ListElement{
            text: "Scanlines"
            obj_string: '{"ambientLight":0.16,"backgroundColor":"#000000","bloom":0.4,"brightness":0.6,"flickering":0.1,"contrast":0.85,"fontName":"TERMINUS_SCALED","fontColor":"#0ccc68","frameName":"SIMPLE_WHITE_FRAME","glowingLine":0,"horizontalSync":0.16,"jitter":0.18,"burnIn":0.22,"staticNoise":0.1,"rasterization":1,"screenCurvature":0.1,"windowOpacity":1,"chromaColor":0,"saturationColor":0,"rbgShift":0,"fontWidth":1.0,"useCustomCommand":false,"graphics":"unix"}'
            builtin: true
        }
        ListElement{
            text: "Amber"
            obj_string: '{"ambientLight":0.16,"backgroundColor":"#000000","bloom":0.65,"brightness":0.5,"flickering":0.1,"contrast":0.85,"fontName":"IBM_DOS","fontColor":"#ff8100","frameName":"SIMPLE_WHITE_FRAME","glowingLine":0,"horizontalSync":0.16,"jitter":0.18,"burnIn":0.2,"staticNoise":0.1,"rasterization":0,"screenCurvature":0.1,"windowOpacity":1,"chromaColor":0,"saturationColor":0,"rbgShift":0,"fontWidth":1.0,"useCustomCommand":false,"graphics":"pc"}'
            builtin: true
        }
        ListElement{
            text: "Pixelated"
            obj_string: '{"ambientLight":0.16,"backgroundColor":"#000000","bloom":0,"brightness":0.5,"flickering":0.2,"contrast":0.85,"fontName":"TERMINUS_SCALED","fontColor":"#ffffff","frameName":"ROUGH_BLACK_FRAME","glowingLine":0,"horizontalSync":0.2,"jitter":0,"burnIn":0.22,"staticNoise":0.19,"rasterization":2,"screenCurvature":0.05,"windowOpacity":1,"chromaColor":0,"saturationColor":0,"rbgShift":0,"fontWidth":1.0,"useCustomCommand":false,"graphics":"unix"}'
            builtin: true
        }
        ListElement{
            text: "Tiled"
            obj_string: '{"ambientLight":0.04,"backgroundColor":"#000000","bloom":0.03,"brightness":1.0,"flickering":0.07,"contrast":0.85,"fontName":"IBM_DOS","fontColor":"#ffffff","frameName":"ROUGH_BLACK_FRAME","glowingLine":0,"horizontalSync":0.05,"jitter":0,"burnIn":0.0,"staticNoise":0.0,"rasterization":2,"screenCurvature":0.05,"windowOpacity":1,"chromaColor":1.0,"saturationColor":0,"rbgShift":0,"fontWidth":1.0,"useCustomCommand":false,"graphics":"tiles"}'
            builtin: true
        }
        //ListElement{
        //    text: "Default Scanlines"
        //    obj_string: '{"ambientLight":0.16,"backgroundColor":"#000000","bloom":0.4,"brightness":0.5,"flickering":0.1,"contrast":0.85,"fontName":"TERMINUS_SCALED","fontColor":"#00ff5b","frameName":"SIMPLE_WHITE_FRAME","glowingLine":0,"horizontalSync":0.14,"jitter":0.11,"burnIn":0.2,"staticNoise":0.05,"rasterization":1,"screenCurvature":0.1,"windowOpacity":1,"chromaColor":0,"saturationColor":0,"rbgShift":0,"fontWidth":1.0,"useCustomCommand":false,"customCommand":""}'
        //    builtin: true
        //}
        //ListElement{
        //    text: "Apple ]["
        //    obj_string: '{"ambientLight":0.16,"backgroundColor":"#000000","bloom":0.5,"brightness":0.5,"flickering":0.2,"contrast":0.85,"fontName":"APPLE_II","fontColor":"#2fff91","frameName":"SIMPLE_WHITE_FRAME","glowingLine":0,"horizontalSync":0.16,"jitter":0.1,"burnIn":0.33,"staticNoise":0.08,"rasterization":1,"screenCurvature":0.18,"windowOpacity":1,"chromaColor":0,"saturationColor":0,"rbgShift":0,"fontWidth":1.0,"useCustomCommand":false,"customCommand":""}'
        //    builtin: true
        //}
        //ListElement{
        //    text: "Vintage"
        //    obj_string: '{"ambientLight":0.5,"backgroundColor":"#000000","bloom":0.4,"brightness":0.5,"flickering":0.9,"contrast":0.80,"fontName":"COMMODORE_PET","fontColor":"#00ff3e","frameName":"ROUGH_BLACK_FRAME","glowingLine":0,"horizontalSync":0.42,"jitter":0.4,"burnIn":0.38,"staticNoise":0.2,"rasterization":1,"screenCurvature":0.1,"windowOpacity":1,"chromaColor":0,"saturationColor":0,"rbgShift":0,"fontWidth":1.0,"useCustomCommand":false,"customCommand":""}'
        //    builtin: true
        //}
        //ListElement{
        //    text: "IBM Dos"
        //    obj_string: '{"ambientLight":0.16,"backgroundColor":"#000000","bloom":0.4,"brightness":0.5,"flickering":0.07,"contrast":0.85,"fontName":"IBM_DOS","fontColor":"#ffffff","frameName":"SIMPLE_WHITE_FRAME","glowingLine":0,"horizontalSync":0,"jitter":0.16,"burnIn":0.15,"staticNoise":0.03,"rasterization":0,"screenCurvature":0.1,"windowOpacity":1,"chromaColor":1,"saturationColor":0,"rbgShift":0.35,"fontWidth":1.0,"useCustomCommand":false,"customCommand":""}'
        //    builtin: true
        //}
        //ListElement{
        //    text: "IBM 3278"
        //    obj_string: '{"ambientLight":0.1,"backgroundColor":"#000000","bloom":0.15,"brightness":0.5,"flickering":0,"contrast":0.85,"fontName":"IBM_3278","fontColor":"#0ccc68","frameName":"SIMPLE_WHITE_FRAME","glowingLine":0,"horizontalSync":0,"jitter":0,"burnIn":0.3,"staticNoise":0,"rasterization":0,"screenCurvature":0.1,"windowOpacity":1,"chromaColor":0,"saturationColor":0,"rbgShift":0,"fontWidth":1.0,"useCustomCommand":false,"customCommand":""}'
        //    builtin: true
        //}
        //ListElement{
        //    text: "Transparent Green"
        //    obj_string: '{"ambientLight":0.2,"backgroundColor":"#000000","bloom":0.45,"brightness":0.5,"flickering":0.20,"contrast":0.85,"fontName":"TERMINUS_SCALED","fontColor":"#0ccc68","frameName":"NO_FRAME","glowingLine":0,"horizontalSync":0.1,"jitter":0.20,"burnIn":0.12,"staticNoise":0.20,"rasterization":0,"screenCurvature":0.05,"windowOpacity":0.60,"chromaColor":0,"saturationColor":0,"rbgShift":0,"fontWidth":1.0,"useCustomCommand":false,"customCommand":""}'
        //    builtin: true
        //}
    }

    function getProfileIndexByName(name) {
        for (var i = 0; i < profilesList.count; i++) {
            if(profilesList.get(i).text === name)
                return i;
        }
        return -1;
    }

    Component.onCompleted: {
        // Manage the arguments from the QML side.
        var args = Qt.application.arguments;
        if (args.indexOf("--verbose") !== -1) {
            verbose = true;
        }
        if (args.indexOf("--default-settings") === -1) {
            loadSettings();
        }

        loadCustomProfiles();

        var profileArgPosition = args.indexOf("--profile");
        if (profileArgPosition !== -1) {
            var profileIndex = getProfileIndexByName(args[profileArgPosition + 1]);
            if (profileIndex !== -1)
                loadProfile(profileIndex);
            else
                console.log("Warning: selected profile is not valid; ignoring it");
        }

        //todo: mdk not working
        /*if (args.indexOf("--fullscreen") !== -1) {
            fullscreen = true;
            showMenubar = false;
        }*/

        initializedSettings();
    }
    Component.onDestruction: {
        storeSettings();
        storeCustomProfiles();
        //storage.dropSettings(); //DROPS THE SETTINGS!.. REMEMBER TO DISABLE ONCE ENABLED!!
    }

    // VARS ///////////////////////////////////////////////////////////////////

    property Label _sampleLabel: Label {
        text: "100%"
    }
    property real labelWidth: _sampleLabel.width
}
