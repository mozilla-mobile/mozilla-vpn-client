import QtQuick 2.0
import "qrc:/addons/group_demo/fs/script.mjs" as TicTacToe

Rectangle {
  color: "red"

  Text {
    id: title

    anchors.top: parent.top
    anchors.left: parent.left

    width: parent.width
    font.pixelSize: 20
    horizontalAlignment: Text.AlignHCenter

    //% "Loading"
    text: qsTrId("tictactoe.title.loading")
  }

  Grid {
    id: grid
    property bool playing: false
    property var matrix: []

    columns: 3
    spacing: 2

    anchors.top: title.bottom
    anchors.left: parent.left
    width: parent.width

    function gameOver(state) {
      switch(state) {
        case 1:
          //% "You won!"
          title.text = qsTrId("tictactoe.title.humanWon")
          break;
        case -1:
          //% "I won!"
          title.text = qsTrId("tictactoe.title.aiWon")
          break;
        case 0:
          //% "Draw!"
          title.text = qsTrId("tictactoe.title.draw")
      }
    }

    Repeater {
      model: 9
      Rectangle {
        id: rect
        states: [
          State {
            name: "unplayed"
            PropertyChanges {
              target: rect
              color: "gray"
            }
          },
          State {
            name: "playedHuman"
            PropertyChanges {
              target: rect
              color: "blue"
            }
            PropertyChanges {
              target: text
              text: "X"
            }
          },
          State {
            name: "playedAI"
            PropertyChanges {
              target: rect
              color: "red"
            }
            PropertyChanges {
              target: text
              text: "Y"
            }
          },
          State {
            name: "winnerHuman"
            PropertyChanges {
              target: rect
              color: "yellow"
            }
            PropertyChanges {
              target: text
              text: "X"
            }
          },
          State {
            name: "winnerAI"
            PropertyChanges {
              target: rect
              color: "yellow"
            }
            PropertyChanges {
              target: text
              text: "Y"
            }
          }
        ]

        width: parent.width / 3
        height: parent.width / 3
        state: "unplayed"

        Text {
          id: text
          anchors.fill: parent
          font.pixelSize: parent.height - 5
          horizontalAlignment: Text.AlignHCenter
          text: ""
        }

        MouseArea {
          anchors.fill: parent
          onClicked: {
            if (grid.playing && parent.state === "unplayed") {
              parent.state = "playedHuman"
              if (!TicTacToe.checkGame(grid, true)) {
                TicTacToe.play(grid)
              }
            }
          }
        }

        Component.onCompleted: {
          grid.matrix.push(this)
          if (grid.matrix.length === 9) {
            //% "Let's play!"
            title.text = qsTrId("tictactoe.title.playing")
            grid.playing = true;
            if (Math.random() < 0.5) TicTacToe.play(grid);
          }
        }
      }
    }
  }
}
