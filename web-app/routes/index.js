var express = require('express');
var fs = require('fs');
var util = require('util');
var router = express.Router();
var expressValidator = require('express-validator');  //Added by Adrian to get it to work
router.use(expressValidator());  //Added by Adrian to get it to work
var dns = require('dns');

var shell = require('shelljs');    // install by run 'npm install shelljs' in directory web-app
var network = require('network');  // install by run 'npm install network' in directory web-app

// Module variables
var ip_address, netmask, gateway_ip, dns_ip1, dns_ip2;


/* Add route for logs page */
router.get('/logs', function(req, res, next) {
 
   var logFiles = [];
   var subArray = null;
			
	fs.readdir('/home/debian/web-app/public/logs', function(err,files) {
		if (err) {
			res.render('error', { message: err });
		} else {
			files.sort();
			/* console.log(files);  */
			var index = 0;
			for (index = 0; index < files.length; index++) {
				logFiles.push(files[index]);
			}
			
			//console.log(logFiles); 

			res.render('logs', {logFiles:logFiles});
		}
	});

});


/* Add route for general page */
router.get('/general', function(req, res, next) {
  // Read data from json file
  var jsonString = fs.readFileSync('../bridge-app/settings.json'); 
  var jsonObj = JSON.parse(jsonString);
  //console.log(jsonObj);

  res.render('general', { unit: jsonObj.self_id,
                          site: jsonObj.site_name,
                          serial: jsonObj.serial_number,
		          alarm: 'mk1',
                          logmode: 'normal',
		          commmode: 'ethernet' });
});

router.post('/general', function(req, res, next) {
  /* Check fields */
  req.checkBody('site', 'Site required').notEmpty();
  req.checkBody('serial', 'Serial required').notEmpty();

  // Run validators
  req.getValidationResult().then(function(result) {
    if (!result.isEmpty()) {
      // Polpulate name of invalid field
      var param = result.useFirstErrorOnly().array()[0].param;
      if(param == "site")
      {
         param = "Site Name";
      }
      else
      {
	 param = "Serial Number";
      }
      res.render('error', { message: "Value of '" +  param + "' was invalid!" });
      return;
    }
    else 
    {
      // First, read data from json file
      var jsonString = fs.readFileSync('../bridge-app/settings.json');
      var jsonObj = JSON.parse(jsonString);

      // Update new value from GUI
      jsonObj.self_id = parseInt(req.body.unit, 10);
      jsonObj.site_name = req.body.site;
      jsonObj.serial_number = req.body.serial;      

      // Save new data to json file
      var newJsonString = JSON.stringify(jsonObj, null, 2);  
      fs.writeFileSync('../bridge-app/settings.json', newJsonString);

      res.render('success', { message : 'General settings have updated!' });
    }
  });
});

/* Add route for network settings page */
router.get('/network', function(req, res, next) {
  // Read data from json file
  var jsonString = fs.readFileSync('../bridge-app/settings.json');
  var jsonObj = JSON.parse(jsonString);
  //console.log(jsonObj);

  dns_ip1 = "";
  dns_ip2 = "";
  // Get DNS IP address
  var dnsServers = dns.getServers();
  if(dnsServers.length == 1)
    dns_ip1 = dnsServers[0];
  else if(dnsServers.length == 2) {
    dns_ip1 = dnsServers[0];
    dns_ip2 = dnsServers[1];
  }

  // Get current NTP servers from configuration file of systemd-timesyncd
  var ntpservers = '';
  var ntpConfig = fs.readFileSync('/etc/systemd/timesyncd.conf', 'utf8');
  var lines = ntpConfig.split('\n');
  // Find row that holds NTP servers (row begins with NTP=)
  for(var i = 0; i < lines.length; i++) {
    if(lines[i].indexOf('NTP=') == 0) {
      ntpservers = lines[i].substr(4);
    }
  }

  network.get_active_interface(function(err, obj) {
    //console.log(obj.name);
    //console.log(obj.ip_address);
    //console.log(obj.netmask);
    //console.log(obj.gateway_ip);

    // Holds network settings
    ip_address = obj.ip_address;
    netmask = obj.netmask;
    gateway_ip = obj.gateway_ip;

    res.render('network', { ipaddress: ip_address,
                            subnet: netmask,
			    gateway: gateway_ip,
			    dns1: dns_ip1,
			    dns2: dns_ip2,
 			    port: '33689',
			    interval: '10000',
			    ntpservers: ntpservers,
  			    enable0: (jsonObj.units[0].unit_type == "master" ? 'true' : 'false'),
			    unit0: jsonObj.units[0].unit_ipaddr,
 			    enable1: (jsonObj.units[1].unit_type == "slave" ? 'true' : 'false'),
			    unit1: jsonObj.units[1].unit_ipaddr,
			    enable2: (jsonObj.units[2].unit_type == "slave" ? 'true' : 'false'),
                            unit2: jsonObj.units[2].unit_ipaddr,
			    enable3: (jsonObj.units[3].unit_type == "slave" ? 'true' : 'false'),
                            unit3: jsonObj.units[3].unit_ipaddr,
			    enable4: (jsonObj.units[4].unit_type == "slave" ? 'true' : 'false'),
                            unit4: jsonObj.units[4].unit_ipaddr,
			    enable5: (jsonObj.units[5].unit_type == "slave" ? 'true' : 'false'),
                            unit5: jsonObj.units[5].unit_ipaddr,
			    enable6: (jsonObj.units[6].unit_type == "slave" ? 'true' : 'false'),
                            unit6: jsonObj.units[6].unit_ipaddr,
			    enable7: (jsonObj.units[7].unit_type == "slave" ? 'true' : 'false'),
                            unit7: jsonObj.units[7].unit_ipaddr,
			    enable8: (jsonObj.units[8].unit_type == "slave" ? 'true' : 'false'),
                            unit8: jsonObj.units[8].unit_ipaddr,
			    enable9: (jsonObj.units[9].unit_type == "submaster" ? 'true' : 'false'),
			    unit9: jsonObj.units[9].unit_ipaddr });
    });
});

router.post('/network', function(req, res, next) {
  //console.log(req.body);

  /* Check fields */
  req.checkBody('ipaddress', 'IP address format required').isIP();
  req.checkBody('subnet', 'IP address format required').isIP();
  req.checkBody('gateway', 'IP address format required').isIP();
  req.checkBody('dns1', 'IP address format required').isIP();
  req.checkBody('dns2', 'IP address format required').isIP();
  req.checkBody('unit0', 'IP address format required').isIP();

  // Run the validators
  req.getValidationResult().then(function(result) {
    if (!result.isEmpty()) {
      // Polpulate name of invalid field
      var param = result.useFirstErrorOnly().array()[0].param;
      if(param == "ipaddress")
      {
         param = "IP Address";
      }
      else if(param == "subnet")
      {
         param = "Subnet";
      }
      else if(param == "gateway")
      {
         param = "Gateway";
      }
      else if(param == "dns1")
      {
         param = "DNS1";
      }
      else if(param == "dns2")
      {
         param = "DNS2";
      }
      else if(param == "unit0")
      {
         param = "Master IP Address";
      }
      else if(param == "unit1")
      {
         param = "Slave1 IP Address";
      }
      else if(param == "unit2")
      {
         param = "Slave2 IP Address";
      }
      else if(param == "unit3")
      {
         param = "Slave3 IP Address";
      }
      else if(param == "unit4")
      {
         param = "Slave4 IP Address";
      }
      else if(param == "unit5")
      {
         param = "Slave5 IP Address";
      }
      else if(param == "unit6")
      {
         param = "Slave6 IP Address";
      }
      else if(param == "unit7")
      {
         param = "Slave7 IP Address";
      }
      else if(param == "unit8")
      {
         param = "Slave8 IP Address";
      }
      else if(param == "unit9")
      {
         param = "Sub-Master IP Address";
      }

      res.render('error', { message: "Value of '" +  param + "' was invalid!" });
      return;
    }
    else 
    {
      /////////////////////////////////////////////////////////////////////////////////////////////
      // Update NTP Servers
      /////////////////////////////////////////////////////////////////////////////////////////////

      // First, get current NTP servers from configuration file of systemd-timesyncd
      var ntpConfig = fs.readFileSync('/etc/systemd/timesyncd.conf', 'utf8');
      var lines = ntpConfig.split('\n');
      // Find row that holds NTP servers (row begins with NTP=)
      for(var i = 0; i < lines.length; i++) {
        if(lines[i].indexOf('NTP=') == 0) {
          lines[i] = 'NTP=' + req.body.ntpservers;
          console.log(lines[i]);
        }
      }
      // Write new string array to file
      fs.writeFileSync('/etc/systemd/timesyncd.conf', lines.join('\n'));


      /////////////////////////////////////////////////////////////////////////////////////////////
      // Update JSON/Text file
      /////////////////////////////////////////////////////////////////////////////////////////////

      // First, read data from json file
      var jsonString = fs.readFileSync('../bridge-app/settings.json');
      var jsonObj = JSON.parse(jsonString);

      if(req.body.check0 == "on")
        jsonObj.units[0].unit_type = "master";
      else
        jsonObj.units[0].unit_type = "unused";
      jsonObj.units[0].unit_ipaddr = req.body.unit0;

      if(req.body.check1 == "on")
        jsonObj.units[1].unit_type = "slave";
      else
        jsonObj.units[1].unit_type = "unused";
      jsonObj.units[1].unit_ipaddr = req.body.unit1;

      if(req.body.check2 == "on")
        jsonObj.units[2].unit_type = "slave";
      else
        jsonObj.units[2].unit_type = "unused";
      jsonObj.units[2].unit_ipaddr = req.body.unit2;

      if(req.body.check3 == "on")
        jsonObj.units[3].unit_type = "slave";
      else
        jsonObj.units[3].unit_type = "unused";
      jsonObj.units[3].unit_ipaddr = req.body.unit3;

      if(req.body.check4 == "on")
        jsonObj.units[4].unit_type = "slave";
      else
        jsonObj.units[4].unit_type = "unused";
      jsonObj.units[4].unit_ipaddr = req.body.unit4;

      if(req.body.check5 == "on")
        jsonObj.units[5].unit_type = "slave";
      else
        jsonObj.units[5].unit_type = "unused";
      jsonObj.units[5].unit_ipaddr = req.body.unit5;

      if(req.body.check6 == "on")
        jsonObj.units[6].unit_type = "slave";
      else
        jsonObj.units[6].unit_type = "unused";
      jsonObj.units[6].unit_ipaddr = req.body.unit6;

      if(req.body.check7 == "on")
        jsonObj.units[7].unit_type = "slave";
      else
        jsonObj.units[7].unit_type = "unused";
      jsonObj.units[7].unit_ipaddr = req.body.unit7;

      if(req.body.check8 == "on")
        jsonObj.units[8].unit_type = "slave";
      else
        jsonObj.units[8].unit_type = "unused";
      jsonObj.units[8].unit_ipaddr = req.body.unit8;

      if(req.body.check9 == "on")
        jsonObj.units[9].unit_type = "submaster";
      else
        jsonObj.units[9].unit_type = "unused";
      jsonObj.units[9].unit_ipaddr = req.body.unit9;

      // Save new data to json file
      var newJsonString = JSON.stringify(jsonObj, null, 2);
      fs.writeFileSync('../bridge-app/settings.json', newJsonString);

      //
      // Update network settings
      // 

      // Get name of cable ethernet resource. It can return many resources. Some of them were invalid !!!!!!!!!!!!!!!! 
      var ethResourceDir = shell.ls('/var/lib/connman');
      //console.log(ethResourceDir[0]);
      var ethernetName = '';
      
      if(dns_ip1 != req.body.dns1 || dns_ip2 != req.body.dns2) {
        for(var i = 0; i < ethResourceDir.length; i++) {
  	  if (ethResourceDir[i].indexOf('ethernet') > -1) {
            var command1 = "connmanctl config " + ethResourceDir[i] + " --nameservers " + req.body.dns1 + " " + req.body.dns2;
            //console.log(command1);
            var result1 = shell.exec(command1, {silent:true}); 
            //console.log(result1);
            if(result1.stderr == '') {
              ethernetName = ethResourceDir[i];
              break;
            }
          }
        }
      }

      if(ip_address != req.body.ipaddress || netmask != req.body.subnet || gateway_ip != req.body.gateway) {
        if(ethernetName != '') {
          var command2 = "connmanctl config " + ethernetName + " --ipv4 manual " + req.body.ipaddress + " " + req.body.subnet + " " + req.body.gateway;
          //console.log(command2);
          shell.exec(command2, {silent:true});
        }
        else {
          for(var i = 0; i < ethResourceDir.length; i++) {
            if (ethResourceDir[i].indexOf('ethernet') > -1) {
              var command2 = "connmanctl config " + ethResourceDir[i] + " --ipv4 manual " + req.body.ipaddress + " " + req.body.subnet + " " + req.body.gateway;
              //console.log(command2);
              var result2 = shell.exec(command2, {silent:true});
              if(result2.stderr == '') {
                break;
              }
            }
          }
        }
      }

      res.render('success', { message : 'Network settings have updated!' });
    }
  });
});

/* GET home page. */
router.get('/', function(req, res, next) {
  res.render('index', { title: 'Express' });
});

module.exports = router;
