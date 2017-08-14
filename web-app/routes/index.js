var express = require('express');
var router = express.Router();

router.get("/general", function(req, res, next) {
  res.render('index1', { title: 'Express' });
});

router.get("/network", function(req, res, next) {
  res.render('index2', { title: 'Express' });
});

/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('index1', { title: 'Express' });
});

module.exports = router;
