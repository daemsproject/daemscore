/* 
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
var BrowserAPI = new function () {
    var apiconnected = false;
    var notifyblockfunc;
    var notifytxfunc;
    var notifypeerfunc;

    //We use this function because connect statements resolve their target once, imediately
    //not at signal emission so they must be connected once the jsinterface object has been added to the frame
    //! <!--  [ connect slots ] -->
    this.connectSlots = function ()
    {
        if (!apiconnected) {
            apiconnected = true;
            jsinterface.feedback.connect(this, this.feedback);
            //jsinterface.notify.connect(this,notify);
            //jsinterface.updateWalletFeedback.connect(this, MyWallet.updateWalletFeedback);
        }
    }
    //! <!--  [ connect slots ] -->

    this.feedback = function (feedbackjson, func) {
        func($.parseJSON(feedbackjson));
        //setStatus('Idle');
        //document.getElementById('testdiv').innerHTML = a;        
    }
    function test() {
        document.getElementById('testdiv').innerHTML = jsinterface.str;
        this.connectSlots();
        jsinterface.test();
    }
    this.call = function (cmd, datajson, successfunc, errorfunc, async) {
        if (async) {
            this.connectSlots();
            jsinterface.jscallasync(cmd, datajson, successfunc, errorfunc);
            return;
        }
        var jsreply;
        var jsreplyjson = jsinterface.jscall(cmd, datajson);
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
            errorfunc(jsreply.error);
            return;
        }
//        console.log("browserapi.call:" + jsreply);
        successfunc(jsreply);
    };
    this.getAccountID = function () {
        var jsreply;
        this.call("getmainid", "", function (a) {
            jsreply = a;
        }, function (b) {
            return null
        });
        console.log("accountid:" + jsreply);
        return jsreply;
    };
    this.getIDs = function (id) {
        var jsreply;
        var data = [id];
        this.call("getidlist", JSON.stringify(data), function (a) {
            jsreply = a;
        }, function (b) {
            console.log(b);
            jsreply = null;
        });
        console.log("ids:" + jsreply.ids[0]);
        return jsreply.ids;
    };
    this.get_history = function (id, success, error, offset, number) {
        var data = [];
        data[0] = id;
        data[1] = (number && isNaN(offset)) ? number : 10000;
        data[2] = (offset && isNaN(offset)) ? offset : 0;
        this.call("listtransactions", JSON.stringify(data), function (result) {
            console.log(result);
            if (success)
                success(result);
        }, function (e) {
            if (error)
                error(e);
        });
    }
    this.regNotifyBlocks = function (blocksfunc) {
        var data = [{blocks: blocksfunc}];
        this.call("regnotify", JSON.stringify(data), null, null, true);
    };
    this.regNotifyTxs = function (txfunc, ids) {
        var data = [{txs: txfunc, ids: ids}];
        this.call("regnotify", JSON.stringify(data), null, null, true);
    };
    this.regNotifyPeers = function (peerfunc) {
        var data = [{peers: peerfunc}];
        this.call("regnotify", JSON.stringify(data), null, null, true);
    }
    this.regNotifyAccount = function (accountfunc) {
        var data = [{account: accountfunc}];
        this.call("regnotify", JSON.stringify(data), null, null, true);
    }
    this.getInfo = function () {
        var r;
        this.call("getinfo", "", function (r1) {
            r = JSON.stringify(r1);
        });
        return r;
    }
    this.getBlockCount = function () {
        var r;
        this.call("getblockcount", "", function (r1) {
            r = JSON.stringify(r1);
        });
        return r;
    }
    this.decodeContent = function (c) {
        var r;
        var d = [c];
        this.call("decodecontent", JSON.stringify(d), function (r1) {
            r = JSON.stringify(r1);
        }, function (e) {
            r = e;
        });
        return r;
    }
    this.createContent = function (c) {
        var r;
        var d = [c];
        this.call("createcontent", JSON.stringify(d), function (r1) {
            r = JSON.stringify(r1);
        }, function (e) {
            r = e;
        });
        return r;
    }
    this.getContentByLink = function (c) {
        var r;
        var d = [c,4];
        this.call("getcontentbylink", JSON.stringify(d), function (r1) {
            r = r1;
        }, function (e) {
            r = e;
        });
        return r;
    }


}

