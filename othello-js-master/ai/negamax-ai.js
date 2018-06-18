(function () {
  var O = othello;

  function ajax(data) {
      var xmlHttp = new XMLHttpRequest();
      xmlHttp.onreadystatechange = function() {
          if (this.readyState === 4 && this.status === 200) {
              var response = this.response;
              if (typeof(response) === 'string') {
                  resoinse = JSON.parse(response);
              }
              // ...
          }
      }
      data.ai = "negamax";
      xmlHttp.open("POST", 'http://localhost:8080/', false); // true:非同期、false:同期
      xmlHttp.send(JSON.stringify(data));
      return xmlHttp.responseText;
  }

  O.registerAI({
    findTheBestMove: function (gameTree) {
      var res = ajax(gameTree);
      return gameTree.moves[res];
    }
  });
})();
// vim: expandtab softtabstop=2 shiftwidth=2 foldmethod=marker
