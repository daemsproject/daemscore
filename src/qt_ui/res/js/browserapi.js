/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var BrowserAPI = new function () {
    var apiconnected = false;
    var notifyblockfunc;
    var notifytx = {func: "", ids: []};
    var notifypeerfunc;
    var notifyaccountfunc;
    var callIDs = [];
    this.connectSlots = function ()
    {
        if (!apiconnected) {
            apiconnected = true;
            jsinterface.feedback.connect(this, this.feedback);
            jsinterface.notify.connect(this, this.notify);
            //jsinterface.updateWalletFeedback.connect(this, MyWallet.updateWalletFeedback);
        }
    };
    //! <!--  [ connect slots ] -->

    this.feedback = function (feedbackjson, func) {
        //console.log(feedbackjson);
        //console.log(func);
        //var a=func;
        var msg = $.parseJSON(feedbackjson);
        //console.log(msg);
        if (msg.error)
            msg = msg.error;
        //console.log(msg);
        if (msg.message)
            msg = msg.message;
        //console.log(msg);
        var cmd = 'var a=' + func + ';a("' + msg + '")';
        console.log(cmd);
        eval(cmd);
        //a($.parseJSON(feedbackjson));
        //console.log(1);
        //setStatus('Idle');
        //document.getElementById('testdiv').innerHTML = a;        
    }
    this.notify = function (notifyjson) {
        console.log("notify:" + notifyjson);
        var data = $.parseJSON(notifyjson);
        var cmd;
        if (!data | !data.type)
            return;
        switch (data.type) {
            case "block":
                cmd = 'var a=' + notifyblockfunc + ';a(data);';
                break;
            case "tx":
                if (!notifytx.func || !data.tx || !data.tx.ids)
                    return;
                var found = false;
                for (var i in notifytx.ids) {
                    console.log("notifytx id:" + notifytx.ids[i]);
                    for (var j in data.tx.ids) {
                        if (data.tx.ids[j] == notifytx.ids[i]) {
                            console.log("notifytx found:" + data.tx.ids[j]);
                            found = true;
                            break;
                        }
                    }
                    if (found)                        break;
                }
                if (!found)                    return;
                cmd = 'var a=' + notifytx.func + ';a(data);';
                break;
            case "accountSwitch":
                cmd = 'var a=' + notifyaccountfunc + ';a(data);';
        }
        console.log(cmd);
        eval(cmd);
    }
//    function test() {    
//        document.getElementById('testdiv').innerHTML = jsinterface.str;        
//        this.connectSlots();
//        jsinterface.test();
//    }
    this.call = function (cmd, datajson, successfunc, errorfunc, async) {
        this.connectSlots();
        if (async) {
            var callID = jsinterface.jscallasync(cmd, JSON.stringify(datajson), successfunc, errorfunc);
            callIDs.push(callID);
            console.log(callID);
            return;
        }
        var jsreply;
        var jsreplyjson = jsinterface.jscall(cmd, JSON.stringify(datajson));
//        console.log("browserapi.call:" + jsreplyjson);
        try {
            jsreply = $.parseJSON(jsreplyjson);
        }
        catch (e) {
            errorfunc(jsreplyjson);
        }
//        console.log("browserapi.call:" + jsreply);
        if (!jsreply) {
            errorfunc("api error");
            return;
        }
        if (jsreply.error) {
            if (jsreply.error.message)
                errorfunc(jsreply.error.message);
            else
                errorfunc(jsreply.error);
            return;
        }
//        console.log("browserapi.call:" + jsreply);
        successfunc(jsreply);
    };
    this.icall=function(cmd,datajson){
        var jsreply;
        var jsreplyjson = jsinterface.jscall(cmd, JSON.stringify(datajson));
        //console.log("browserapi.call:" + jsreplyjson);
        if (!jsreplyjson) {
            return null;
        }
        try {
            jsreply = $.parseJSON(jsreplyjson);
        }
        catch (e) {
            console.log("icall error:"+cmd+e);
            return false;
        }
        if (jsreply.error) {
            if (jsreply.error.message)
                console.log("icall error:" + cmd + jsreply.error.message);
            else
                console.log("icall error:" + cmd + jsreply.error);
            return false;
        }
        return jsreply;
    };
    this.getAccountID = function () {return this.icall("getmainid", []);};
    this.getIDs = function (id) {
        var jsreply = this.icall("getidlist", [id]);
        return jsreply.ids;
    };
    this.requestPayment = function (fromIDs, toID, amount, content, success, error) {
        var data = [];
        var request = {};
        if (!fromIDs) {
            error("sender account id empty");
            return;
        }
        if (!Object.prototype.toString.call(fromIDs) == '[object Array]')
            fromIDs = [fromIDs];
        if (!toID) {
            error("recepient account id empty");
            return;
        }
        if (isNaN(amount)) {
            error("recepient amount is not number");
            return;
        }
        if (!amount) {
            error("recepient amount not set");
            return;
        }
        if (amount < 0) {
            error("recepient amount less than zero");
            return;
        }
        request.ids = fromIDs;
        request.vout = [];
        var vout = {};
        vout.id = toID;
        vout.amount = amount;
        if (content)
            vout.content = content;
        request.vout[0] = vout;
        data[0] = request;
        console.log(JSON.stringify(data));
        this.call("requestpayment", data, success, error, false);

    }
    this.get_history = function (id, success, error, offset, number) {
        var data = [];
        data[0] = id;
        data[1] = (number && isNaN(offset)) ? number : 10000;
        data[2] = (offset && isNaN(offset)) ? offset : 0;
        this.call("listtransactions", data, function (result) {
            if (success)
                success(result);
        }, function (e) {
            if (error)
                error(e);
        });

    }

    this.regNotifyBlocks = function (blocksfunc) {        notifyblockfunc = blocksfunc;  };
    this.regNotifyTxs = function (txfunc, ids) {notifytx.func = txfunc;notifytx.ids = ids;};
    this.regNotifyPeers = function (peerfunc) {}
    this.regNotifyAccount = function (accountfunc) {        notifyaccountfunc=accountfunc;    }
    this.getInfo = function () {return this.icall("getinfo", "")};
    this.getBlockCount = function () {return JSON.stringify(this.icall("getblockcount", ""));};
    this.getContentByLink = function (c) {                return this.icall("getcontentbylink", [c, 6]);    };
    this.setFollow = function (a)  {        return this.icall("setfollow",[[a]]);    };

    this.getFollowed = function () {        return this.icall("getfollowed",[]);    };

    this.getContents = function (fbh, blkc, fAsc, addrs) {
        fbh = typeof fbh !== 'undefined' ? fbh : 0;
        blkc = typeof blkc !== 'undefined' ? blkc : 10;
        fAsc = typeof fAsc !== 'undefined' ? fAsc : true;
        addrs = typeof addrs !== 'undefined' ? addrs : [];
        var r;
        var fcc = ["CC_FILE_P", "CC_TEXT_P", "CC_LINK_P", "CC_LINK"];
        var d = [{"fbh": fbh, "maxc": 20, "maxb": 3000000, "firstcc": fcc, "cformat": 6, "fAsc": fAsc, "mincsize": 3, "blkc": blkc}, addrs];

        return this.icall("getcontents", d);
    };
    this.testGetContents = function (fbh) {
        var fcc = ["CC_FILE_P", "CC_TEXT_P", "CC_LINK_P", "CC_LINK"];
        console.log(fbh);
        var d = [{"fbh": fbh, "maxc": 20, "maxb": 3000000, "firstcc": fcc, "cformat": 6, "fAsc": false, "mincsize": 3, "blkc": 10}];
        return this.icall("getcontents", d);
    };
    this.getMessages = function (idsLocal, idsForeign, directionFilter, fIncludeMempool, fLinkOnly, offset, number, success, error) {
        var d = {};
        if (!idsLocal){
            if (error)        error("no local IDs provided");
            return false;
        }
        //never collects message to idslocal itself

        if (Object.prototype.toString.call(idsLocal) != '[object Array]')
            idsLocal=[idsLocal];        
        //if idsForeign is provided, only gets messages between local & foreign.
        if (idsForeign)           d.IDsForeign = idsForeign;
        //direction filter:        0:both in/out        1:incoming only        2:outputs only
        if (directionFilter)     d.directionFilter = directionFilter;
        //flinkonly:only provids link, don't decrypt
        if (fLinkOnly)          d.fLinkOnly = fLinkOnly;
        if (fIncludeMempool)     d.fincludeMempool = fIncludeMempool;
        if (offset)             d.offset = offset;
        if (number)             d.number = number;        
        var a= this.icall("getmessages", [idsLocal,d]);
        if(a.error){
            if(error)
                if (a.error.message)   error(a.error.message)
                else                    error(a.error);
            return false;
        }
        if(success) success(a);
        return a;
    }
    this.getTxMessages = function (idsLocal,txs,success,error) {
        if (!idsLocal){
            if (error)        error("no local IDs provided");
            return false;
        }
        //never collects message to idslocal itself        
        if (Object.prototype.toString.call(idsLocal) != '[object Array]')            
            idsLocal=[idsLocal];         
        var a= this.icall("gettxmessages", [idsLocal,txs]);
        if(a.error){
            if(error)
                if (a.error.message)   error(a.error.message)
                else                    error(a.error);
            return false;
            }
        if(success) success(a);
        return a;
    }
    this.sendMessage=function(idLocal,idForeign,msg,success,error){
        this.call("sendmessage", [idLocal,idForeign,msg],success,error);
    }


    this.getMiningInfo = function () {return this.icall("getmininginfo", [])}
    this.setGenerate = function (generate, id, kernels,success,error) {
        this.call("setgenerate", [generate,Number(kernels),id,false], function (a) {
            if (success)
                success(a);
        }, function (e) {
            if (error)
                error(e);
        });
    }
    this.read_contacts = function (id) {return this.icall("readcontacts", [id]);};
    this.add_contacts = function (id, contacts) {return this.icall("addcontacts", [id, contact])}
    
    this.createTxByContent = function (hexctt) {
        var accountID = BrowserAPI.getAccountID();
        var IDs = BrowserAPI.getIDs(accountID);
        console.log(IDs);
        BrowserAPI.requestPayment(IDs, "", 0, hexctt, function () {
            console.log("success");
        }, function (e) {
            console.log(e);
        });
    };



    ////////////////////////////////////////////////////////////////////////////////////////////
    //Below is lib funcs
    this.createContentS = function (c) {
        return this.icall("createcontent", [c]);
    };
    this.decodeContent = function (c) {
        return this.icall("decodecontent", [c]);
    };
    this.getContentByString = function (c) {
        return this.icall("getcontentbystring", [c, 6]);
    };
    this.checkNameKey = function (id) {
        return this.icall("isvalidpubkeyaddress", [id]);
    };

    this.getMatureTime = function (locktime) {        return this.icall("getmaturetime",[locktime]);    }
    this.encryptMessages = function (idLocal,msgArr) {return this.icall("encryptmessages", [idLocal,msgArr]);    }
    this.decryptMessages = function (idLocal,msgArr,success,error) {return this.call("encryptmessages", [idLocal,msgArr,false],success,error);    }
    this.areIDsEqual=function(id1,id2){return this.icall("comparebase32",[id1,id2])==0};
    this.createTextContent=function(text,format){
         var u1 = this.icall("encodecontentunit", ["CC_TEXT", text, 2]);
         var u2 = this.icall("encodecontentunit", ["CC_TEXT_ENCODING_UTF8", "", 2]);
         var s = u1.hex + u2.hex;
         return this.icall("encodecontentunit", ["CC_TEXT_P", s, 0]);                
    }
    this.createContentC = function (t, c) {
        console.log("t " + t);
        switch (t)
        {
            case "text/plain":
                var u1 = this.icall("encodecontentunit", ["CC_TEXT", c.data, 2]);
                var u2 = this.icall("encodecontentunit", ["CC_TEXT_ENCODING_UTF8", "", 2]);
                var s = u1.hex + u2.hex;
                var r = this.icall("encodecontentunit", ["CC_TEXT_P", s, 0]);
                break;
            case "image/png":
                var u1 = this.icall("encodecontentunit", ["CC_FILE_NAME", "t", 2]);
                var u2 = this.icall("encodecontentunit", ["CC_FILE_TYPESTRING", t, 1]);
                var u3 = this.icall("encodecontentunit", ["CC_FILE", c.data, 2]);
                var s = u1.hex + u2.hex + u3.hex;
                var r = this.icall("encodecontentunit", ["CC_FILE_P", s, 0]);
                break;
        }
        console.log(u1);
        console.log(u2);
        console.log(r);
        return r;
    };
};

var CLink = new function () {
    var nHeight;
    var nTx;
    var nVout;
    this.setString = function (str) {
        this.nHeight = -1;
        this.nTx = -1;
        this.nVout = -1;
        var pc = str.indexOf(":");
        if (pc >= 0)
            str = str.substring(pc + 1);
        var pfd = str.indexOf(".");
        this.nHeight = parseInt(str.substring(0, pfd));
        var psd = str.indexOf(".", pfd + 1);
        if (psd >= 0) {
            this.nTx = parseInt(str.substring(pfd + 1, psd));
            this.nVout = parseInt(str.substring(psd + 1));
        } else {
            this.nTx = parseInt(str.substring(pfd + 1));
            this.nVout = 0;
        }
        return this;
    };
    this.toString = function () {
        return this.nHeight >= 0 && this.nTx >= 0 ?
                (this.nVout > 0 ? "ccc:" + this.nHeight + "." + this.nTx + "." + this.nVout : "ccc:" + this.nHeight + "." + this.nTx) : "";
    };
    this.toHtmlId = function () {
        return this.nHeight >= 0 && this.nTx >= 0 ?
                (this.nVout > 0 ? this.nHeight + "_" + this.nTx + "_" + this.nVout : this.nHeight + "_" + this.nTx) : "";
    };
};

var CUtil = new function () {
    this.decodeDataUrl = function (s) {
        var r = [];
        var t1 = s.split(";");
        var rctt = t1[1];
        var t2 = t1[0].split(":");
        r["type"] = t2[1];
        var t3 = rctt.split(",");
        r["data"] = t3[1];

        return r;
    };
};
function sleep(n) {
    var start = new Date().getTime();
    while(true)  if(new Date().getTime()-start > n) break;
}

