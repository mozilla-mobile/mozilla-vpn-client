const winPos = [
  [ 0, 1, 2 ],
  [ 3, 4, 5 ],
  [ 6, 7, 8 ],
  [ 0, 4, 8 ],
  [ 2, 4, 6 ],
  [ 0, 3, 6 ],
  [ 1, 4, 7 ],
  [ 2, 5, 8 ],
];

export function checkGame(board, player) {
  const playerState = player ? "playedHuman" : "playedAI";
  for (const p of winPos) {
    if (board.matrix[p[0]].state === playerState &&
        board.matrix[p[1]].state === playerState &&
        board.matrix[p[2]].state === playerState) {
      const winnerState = player ? "winnerHuman" : "winnerAI";
      board.matrix[p[0]].state = winnerState;
      board.matrix[p[1]].state = winnerState;
      board.matrix[p[2]].state = winnerState;

      board.gameOver(player ? 1 : -1);
      board.playing = false;
      return true;
    }
  }

  if (!board.matrix.find(m => m.state === "unplayed")) {
    board.gameOver(0);
    board.playing = false;
    return true;
  }

  return false;
}

export function play(board) {
  let onePosLeftForHuman = [];
  let possibleAIWinPos = [];

  for (const p of winPos) {
    if (board.matrix[p[0]].state !== "playedAI" &&
        board.matrix[p[1]].state !== "playedAI" &&
        board.matrix[p[2]].state !== "playedAI") {
      const posLeft = p.filter(p => board.matrix[p].state === "unplayed").map(p => board.matrix[p]);
      if (posLeft.length === 1) onePosLeftForHuman.push(posLeft[0]);
    }
    if (board.matrix[p[0]].state !== "playedHuman" &&
        board.matrix[p[1]].state !== "playedHuman" &&
        board.matrix[p[2]].state !== "playedHuman") {
      possibleAIWinPos.push(
        p.filter(p => board.matrix[p].state === "unplayed").map(p => board.matrix[p])
      );
    }
  }

  const quickWin = possibleAIWinPos.find(p => p.length <= 1);
  if (quickWin) {
    possibleAIWinPos = [quickWin];
  } else if (onePosLeftForHuman.length) {
    possibleAIWinPos = [onePosLeftForHuman];
  } else if (!possibleAIWinPos.length) {
    possibleAIWinPos = [board.matrix.filter(p => p.state === "unplayed")];
  }

  possibleAIWinPos = possibleAIWinPos.sort((a, b) => a.length - b.length);
  possibleAIWinPos = possibleAIWinPos.filter(a => a.length <= possibleAIWinPos[0].length);
  const pos = possibleAIWinPos[Math.floor(Math.random() * possibleAIWinPos.length)];
  pos[Math.floor(Math.random() * pos.length)].state = "playedAI";
  checkGame(board);
}
