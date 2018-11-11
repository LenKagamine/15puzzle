function toMoves(sol) {
  const MOVES = ["Null", "D", "L", "U", "R"];
  return sol.map(m => MOVES[m]);
}

function Tile(x, y, num, width, height) {
  this.x = x;
  this.y = y;

  const el = document.createElement("div");
  el.className = "tile";
  el.innerHTML = `<div class="inner">${num}</div>`;
  el.style.width = width + "%";
  el.style.height = height + "%";
  el.style.top = y * width + "%";
  el.style.left = x * height + "%";

  this.setPosition = (newX, newY) => {
    this.x = newX;
    this.y = newY;
    el.style.top = newY * width + "%";
    el.style.left = newX * height + "%";
  };

  this.onClick = callback => {
    el.addEventListener("click", callback);
  };

  this.attach = box => {
    box.appendChild(el);
  };
}

function Board(width, height, direction) {
  const blank = {
    x: direction ? width - 1 : 0,
    y: direction ? height - 1 : 0
  };
  const length = width * height;
  const nums = direction
    ? [...Array(length - 1).keys()].map(i => i + 1).concat(0)
    : [...Array(length).keys()];

  const tileWidth = 100 / width;
  const tileHeight = 100 / height;

  const tiles = nums.reduce((acc, num) => {
    if (num === 0) return acc;
    const index = direction ? num - 1 : num;
    const tile = new Tile(
      index % width,
      0 | (index / width),
      num,
      tileWidth,
      tileHeight
    );

    tile.onClick(() => {
      const { x, y } = tile;
      if (blank.x - 1 === x && blank.y === y) {
        this.moveRight();
      } else if (blank.x + 1 === x && blank.y === y) {
        this.moveLeft();
      } else if (blank.y - 1 === y && blank.x === x) {
        this.moveDown();
      } else if (blank.y + 1 === y && blank.x === x) {
        this.moveUp();
      }
    });

    return {
      ...acc,
      [num]: tile
    };
  }, {});

  this.attach = box => {
    box.innerHTML = "";
    Object.values(tiles).forEach(tile => tile && tile.attach(box));

    document.addEventListener("keydown", e => {
      if (e.code === "KeyW" || e.code === "ArrowUp") this.moveUp();
      if (e.code === "KeyD" || e.code === "ArrowRight") this.moveRight();
      if (e.code === "KeyS" || e.code === "ArrowDown") this.moveDown();
      if (e.code === "KeyA" || e.code === "ArrowLeft") this.moveLeft();
    });
  };

  this.moveUp = () => {
    if (blank.y < height - 1) {
      const blankIndex = blank.y * width + blank.x;
      const tileIndex = blankIndex + width;
      const tileNum = nums[tileIndex];
      tiles[tileNum].setPosition(blank.x, blank.y);
      nums[tileIndex] = 0;
      nums[blankIndex] = tileNum;
      blank.y += 1;
    }
  };

  this.moveRight = () => {
    if (blank.x > 0) {
      const blankIndex = blank.y * width + blank.x;
      const tileIndex = blankIndex - 1;
      const tileNum = nums[tileIndex];
      tiles[tileNum].setPosition(blank.x, blank.y);
      nums[tileIndex] = 0;
      nums[blankIndex] = tileNum;
      blank.x -= 1;
    }
  };

  this.moveDown = () => {
    if (blank.y > 0) {
      const blankIndex = blank.y * width + blank.x;
      const tileIndex = blankIndex - width;
      const tileNum = nums[tileIndex];
      tiles[tileNum].setPosition(blank.x, blank.y);
      nums[tileIndex] = 0;
      nums[blankIndex] = tileNum;
      blank.y -= 1;
    }
  };

  this.moveLeft = () => {
    if (blank.x < width - 1) {
      const blankIndex = blank.y * width + blank.x;
      const tileIndex = blankIndex + 1;
      const tileNum = nums[tileIndex];
      tiles[tileNum].setPosition(blank.x, blank.y);
      nums[tileIndex] = 0;
      nums[blankIndex] = tileNum;
      blank.x += 1;
    }
  };

  this.getNums = () => {
    const chunked = [];
    for (let i = 0, y = 0; y < height; y++, i += width) {
      chunked.push(nums.slice(i, i + width));
    }
    return chunked;
  };

  this.applyMoves = moves => {
    let index = 0;

    const applyMove = () => {
      setTimeout(() => {
        const move = moves[index];

        if (move === 1) this.moveDown();
        else if (move === 2) this.moveLeft();
        else if (move === 3) this.moveUp();
        else if (move === 4) this.moveRight();

        if (++index < moves.length) applyMove();
      }, 500);
    };
    applyMove();
  };
}

const board = new Board(4, 4, true);
board.attach(document.getElementById("root"));

const setupBtn = document.getElementById("setup-btn");
const solveBtn = document.getElementById("solve-btn");

Module.addOnPostRun(() => {
  function vec(arr) {
    const vec = new Module.VecInt();
    vec.resize(arr.length, 0);
    for (let i = 0; i < arr.length; i++) {
      vec.set(i, arr[i]);
    }
    return vec;
  }

  function vec2(arr2D) {
    const vec2D = new Module.Vec2Int();
    for (let i = 0; i < arr2D.length; i++) {
      vec2D.push_back(vec(arr2D[i]));
    }
    return vec2D;
  }

  function vec3(arr3D) {
    const vec3D = new Module.Vec3Int();
    for (let i = 0; i < arr3D.length; i++) {
      vec3D.push_back(vec2(arr3D[i]));
    }
    return vec3D;
  }

  function arr(vec) {
    const arr = [];
    const size = vec.size();
    for (let i = 0; i < size; i++) {
      arr.push(vec.get(i));
    }
    return arr;
  }

  const database555Reg = vec3([
    [[1, 2, 3, 0], [5, 6, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]],
    [[0, 0, 0, 4], [0, 0, 7, 8], [0, 0, 11, 12], [0, 0, 0, 0]],
    [[0, 0, 0, 0], [0, 0, 0, 0], [9, 10, 0, 0], [13, 14, 15, 0]]
  ]);
  const database8reg = vec3([[[1, 2, 3], [4, 5, 6], [7, 8, 0]]]);
  const database8rev = vec3([[[0, 1, 2], [3, 4, 5], [6, 7, 8]]]);

  setupBtn.addEventListener("click", () => {
    Module.setup(database555Reg);
    solveBtn.disabled = false;
  });

  solveBtn.addEventListener("click", () => {
    const nums = board.getNums();
    const startBoard = vec2(nums);

    const solution = arr(Module.solve(startBoard));

    board.applyMoves(solution);
  });
});
