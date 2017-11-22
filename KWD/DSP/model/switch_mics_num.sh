case $1 in
  2)
    rm -f /usr/bin/AMLogic_VUI_Solution_Model.awb
    ln -s /usr/bin/AMLogic_VUI_Solution_Model_Gen2_2Mics.awb /usr/bin/AMLogic_VUI_Solution_Model.awb
    ;;
  4)
    rm -f /usr/bin/AMLogic_VUI_Solution_Model.awb
    ln -s /usr/bin/AMLogic_VUI_Solution_Model_Gen2_4Mics.awb /usr/bin/AMLogic_VUI_Solution_Model.awb
    ;;
  6)
    rm -f /usr/bin/AMLogic_VUI_Solution_Model.awb
    ln -s /usr/bin/AMLogic_VUI_Solution_Model_Gen2_6Mics.awb /usr/bin/AMLogic_VUI_Solution_Model.awb
    ;;
  *)
    echo "only 2/4/6 mics is supported"
    exit 1
esac

