#!/bin/bash

APPLEID=appledev@jonof.id.au
APPLEIDPW=@keychain:appledev-altool
PRODUCT=jfduke3d
BUNDLEID=au.id.jonof.$PRODUCT
VERSION=$(date +%Y%m%d)

if [ "$1" = "build" ]; then
    set -xe

    # Clean everything including JFAudioLib's ogg/vorbis builds.
    (cd xcode && xcrun xcodebuild -target all clean)
    rm -rf jfaudiolib/third-party/osx/out

    # Configure code signing.
    cat >xcode/Signing.xcconfig <<EOT
CODE_SIGN_IDENTITY = Developer ID Application
CODE_SIGN_STYLE = Manual
DEVELOPMENT_TEAM = S7U4E54CHC
CODE_SIGN_INJECT_BASE_ENTITLEMENTS = NO
OTHER_CODE_SIGN_FLAGS = --timestamp
EOT

    # Set the build version.
    cat >xcode/Version.xcconfig <<EOT
CURRENT_PROJECT_VERSION = $VERSION
EOT

    # Build away.
    (cd xcode && xcrun xcodebuild -parallelizeTargets -target all -configuration Release)

elif [ "$1" = "notarise" ]; then
    set -xe

    # Zip up the app bundles. Can't use 'zip' because it fscks with Apple's notarisation.
    ditto -c -k --sequesterRsrc xcode/build/Release notarise.zip

    # Send the zip to Apple.
    xcrun altool --notarize-app --file notarise.zip \
        --primary-bundle-id "$BUNDLEID" \
        -u "$APPLEID" -p "$APPLEIDPW"

elif [ "$1" = "notarystatus" ]; then
    if [ -z "$2" ]; then
        set -xe
        xcrun altool --notarization-history 0 -u "$APPLEID" -p "$APPLEIDPW"
    else
        set -xe
        xcrun altool --notarization-info "$2" -u "$APPLEID" -p "$APPLEIDPW"
    fi

elif [ "$1" = "finish" ]; then
    set -xe

    # Clean a previous packagin