if [ "$1" = "debug" ]
then
    echo "Debug mode activated"
    APP="./bin/video-design_debug.app/Contents/MacOS/video-design_debug"
else
    echo "Release mode activate"
    APP="./bin/video-design.app/Contents/MacOS/video-design"
fi

# $APP test.json

# $APP arco-mvt-2.json &
# $APP arco-mvt-2.json &
# $APP arco-mvt-2.json &
# $APP arco-mvt-2.json &
$APP arco-mvt-2.json