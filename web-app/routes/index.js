var express = require('express');
var router = express.Router();

/* Add route for general page */
router.get('/general', function(req, res, next) {
  res.render('index1', { title: 'Express' });
});

/* Add route for network settings page */
router.get('/network', function(req, res, next) {
  res.render('index2', { title: 'Express' });
});

/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('index', { title: 'Express' });
});

module.exports = router;
