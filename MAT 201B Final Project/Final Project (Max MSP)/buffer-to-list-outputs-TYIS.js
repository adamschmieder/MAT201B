outlets = 1;
autowatch = 1;
var buffer = new Buffer("METER");


var list_lbassm_prev = new Array();


var clocker = 0;
var presence = new Array();
var presenceAvg = 0;
var ps = 5;
var presenceVal = 0;

var volume = 0;
var volumeVal = 0;

var lbassm_bang = false;

var ls = 3;
var las = ls - 1;


function bang() {
  var list = new Array(buffer.framecount());

  for (var i = 0; i < buffer.framecount(); i++) {
    list[i] = buffer.peek(0, i);
  }


  lbassm = (list[2] + list[3] + list[4])/3.0;
  //bassm = (list[5] + list[6] + list[7])/3.0;
  //lmidm = (list[8] + list[9] + list[10] + list[11])/4.0;
  //midm = (list[12] + list[13] + list[14])/3.0;
  //umidm = (list[15] + list[16] + list[17] + list[18] + list[19]+ list[20])/6.0;
  //hmidm = (list[21] + list[22] + list[23] + list[24] + list[25] + list[26])/6.0;
  //uhmidm = (list[27] + list[28] + list[29])/3.0;
  //highm = (list[30] + list[31] + list[32] + list[33])/4.0;
  //uhighm = (list[34] + list[35] + list[36])/3.0;


  // Onset Detection

// Bass Mean

	//post(list_bassm_prev[0]);
    //post(list_bassm_prev[2]);

// Low Mids Mean
  

// Mids Mean
  

// Ultra Mids Mean
  
// High-Mids Mean

// Ultra High-Mids Mean
 

// Highs Mean


// Ultra Highs Mean
 
// Low Bass Mean
  if((lbassm - list_lbassm_prev[las]) > 0.07) {
	lbassm_bang = true;
	outlet(0, 'bang');
  }
  else {
    lbassm_bang = false;
  }
    if(list_lbassm_prev.length < ls) {
		list_lbassm_prev.unshift(lbassm);
	}
	else {
		list_lbassm_prev.unshift(lbassm);
		list_lbassm_prev.pop();
	}


  
  //outlet(23, volumeVal);
  //outlet(24, volume);


										
  //outlet(0, list);
  //outlet(1, lbassm);						// 	mean low bass
  //outlet(2, bassm);							// 	mean bass
  //outlet(3, lmidm);							// 	mean low mids
  //outlet(4, midm);							// 	mean mid 
  //outlet(5, umidm);							// 	mean upper mids
  //outlet(6, hmidm);							// 	mean high-mids
  //outlet(7, uhmidm);						// 	mean upper high-mids
  //outlet(8, highm);							// 	mean highs
  //outlet(9, uhighm);						// 	mean upper highs



  //  send list[12] via OSC to Allolib
  //  we have list already in here
  //  range is from 0-124 for FFt
  //  range is from 0-1 of the list
  //  for onset detection, you need history

}