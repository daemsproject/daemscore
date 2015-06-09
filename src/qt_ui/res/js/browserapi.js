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
    //We use this function because connect statements resolve their target once, imediately
    //not at signal emission so they must be connected once the jsinterface object has been added to the frame
    //! <!--  [ connect slots ] -->
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
        console.log(feedbackjson);
        console.log(func);
        //var a=func;
        var msg = $.parseJSON(feedbackjson);
        console.log(msg);
        if (msg.error)
            msg = msg.error;
        console.log(msg);
        if (msg.message)
            msg = msg.message;
        console.log(msg);
        var cmd = 'var a=' + func + ';a("' + msg + '")';
        console.log(cmd);
        eval(cmd);
        //a($.parseJSON(feedbackjson));
        console.log(1);
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
                    if (found)
                        break;
                }
                if (!found)
                    return;
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
            var callID = jsinterface.jscallasync(cmd, datajson, successfunc, errorfunc);
            callIDs.push(callID);
            console.log(callID);
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
            if (jsreply.error.message)
                errorfunc(jsreply.error.message);
            else
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
            return null;
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
        this.call("requestpayment", JSON.stringify(data), success, error, false);
    }
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
    this.getMatureTime = function (locktime) {
        var result;
        var data = [];
        data[0] = locktime;
        this.call("getmaturetime", JSON.stringify(data), function (r) {
            console.log(r);
            result = r;
        }, function () {
        });
        return result;
    }
    this.regNotifyBlocks = function (blocksfunc) {
        notifyblockfunc = blocksfunc;
        //var data=[{blocks:blocksfunc}];
        //this.call("regnotify",JSON.stringify(data),blocksfunc,blocksfunc,true);
    };
    this.regNotifyTxs = function (txfunc, ids) {
        notifytx.func = txfunc;
        notifytx.ids = ids;
        console.log("notifytx registered:" + ids);
        //var data=[{txs:txfunc,ids:ids}];
        //this.call("regnotify",JSON.stringify(data),txfunc,txfunc,true);
    };
    this.regNotifyPeers = function (peerfunc) {
        //var data=[{peers:peerfunc}];
        //this.call("regnotify",JSON.stringify(data),peerfunc,peerfunc,true);
    }
    this.regNotifyAccount = function (accountfunc) {
        notifyaccountfunc=accountfunc;
        //var data = [{account: accountfunc}];
       // this.call("regnotify", JSON.stringify(data), accountfunc, accountfunc, true);
    }
    this.getInfo = function () {
        var r;
        this.call("getinfo", "", function (r1) {
            r = JSON.stringify(r1);
        });
        return r;
    };
    this.getBlockCount = function () {
        var r;
        this.call("getblockcount", "", function (r1) {
            r = JSON.stringify(r1);
        });
        return r;
    };
    this.decodeContent = function (c) {
        var r;
        var d = [c];
        this.call("decodecontent", JSON.stringify(d), function (r1) {
            r = JSON.stringify(r1);
        }, function (e) {
            r = e;
        });
        return r;
    };
    this.createContent = function (c) {
        var r;
        var d = [c];
        this.call("createcontent", JSON.stringify(d), function (r1) {
            r = JSON.stringify(r1);
        }, function (e) {
            r = e;
        });
        return r;
    };
    this.getContentByLink = function (c) {
        var r;
        var d = [c, 6];
        this.call("getcontentbylink", JSON.stringify(d), function (r1) {
            r = r1;
        }, function (e) {
            r = e;
        });
        return r;
    };
    this.setFollow = function (a) {
        var r;
        var d = [[a]];
        this.call("setfollow", JSON.stringify(d), function (r1) {
            r = r1;
        }, function (e) {
            r = e;
        });
        return r;
    };
    this.getFollowed = function () {
        var r;
        var d = [];
        this.call("getfollowed", JSON.stringify(d), function (r1) {
            r = r1;
        }, function (e) {
            r = e;
        });
        return r;
    };

    this.getContents = function (fbh, blkc, fAsc, addrs) {
        fbh = typeof fbh !== 'undefined' ? fbh : 0;
        blkc = typeof blkc !== 'undefined' ? blkc : 10;
        fAsc = typeof fAsc !== 'undefined' ? fAsc : true;
        addrs = typeof addrs !== 'undefined' ? addrs : [];
        var r;
        var fcc = ["CC_FILE_P", "CC_TEXT_P", "CC_LINK_P", "CC_LINK"];
        var d = [{"fbh": fbh, "maxc": 20, "maxb": 3000000, "firstcc": fcc, "cformat": 6, "fAsc": fAsc, "mincsize": 3, "blkc": blkc}, addrs];

        this.call("getcontents", JSON.stringify(d), function (r1) {
            if (r1.length > 0)
                console.log(JSON.stringify(d));
            r = r1;
        }, function (e) {
            r = e;
        });
        return r;
    };
    this.testGetContents = function (fbh) {
        var r;
        var fcc = ["CC_FILE_P", "CC_TEXT_P", "CC_LINK_P", "CC_LINK"];
        console.log(fbh);
        var d = [{"fbh": fbh, "maxc": 20, "maxb": 3000000, "firstcc": fcc, "cformat": 6, "fAsc": false, "mincsize": 3, "blkc": 10}];
        this.call("getcontents", JSON.stringify(d), function (r1) {
            r = r1;
        }, function (e) {
            r = e;
        });
        return r;
    };
    this.getMessages = function (idsLocal, idsForeign, directionFilter,  fIncludeMempool, fLinkOnly,offset, number,success,error) {
        var data=[];
        var d = {};
        if (!idsLocal){
            error("no local IDs provided");
            return;
        }
        //never collects message to idslocal itself
        data[0]=idsLocal;
        if (Object.prototype.toString.call(idsLocal) != '[object Array]')            
            data[0]=[idsLocal];        
        //if idsForeign is provided, only gets messages between local & foreign.
        if (idsForeign)
            d.IDsForeign = idsForeign;
        //direction filter:
        //0:both in/out
        //1:incoming only
        //2:outputs only
        if (directionFilter)
            d.directionFilter = directionFilter;
        //flinkonly:only provids link, don't decrypt
        if (fLinkOnly)
            d.fLinkOnly = fLinkOnly;
        if (fIncludeMempool)
            d.fincludeMempool = fIncludeMempool;
        if (offset)
            d.offset = offset;
        if (number)
            d.number = number;
        data[1]=d;
        this.call("getmessages", JSON.stringify(data), function (m) {
            if(success)
                success(m);
        }, function (e) {
            if (error)
            error(e);
        });

    }
    this.decryptMessages = function (idLocal,msgArr,success,error) {
        //msgArr:[{IDForeign:"",messages:[""]}]
        var data=[];        
        data[0]=idLocal;
        data[1]=msgArr;        
        data[2]=false;
        this.call("encryptmessages", JSON.stringify(data), function (m) {
            if(success)
                success(m);
        }, function (e) {
            if (error)
            error(e);
        });

    }
    this.encryptMessages = function (idLocal,msgArr,success,error) {
        //msgArr:[{IDForeign:"",messages:[""]}]
        var data=[];        
        data[0]=idLocal;
        data[1]=msgArr;                
        this.call("encryptmessages", JSON.stringify(data), function (m) {
            if(success)
                success(m);
        }, function (e) {
            if (error)
            error(e);
        });

    }
    this.getMiningInfo = function (success, error) {
        this.call("getmininginfo", "[]", function (info) {
            if (success)
                success(info);
        }, function (e) {
            if (error)
                error(e);
        });
    }
    this.setGenerate = function (generate, id, kernels, success, error) {
        var data = [];
        //var str={};
        //str.generate=generate;
        //str.id=id;
        //str.kernels=kernels;
        data[0] = generate;
        data[1] = Number(kernels);
        data[2] = id;
        data[3] = false;
        console.log(JSON.stringify(data));
        this.call("setgenerate", JSON.stringify(data), function (a) {
            if (success)
                success(a);
        }, function (e) {
            if (error)
                error(e);
        });
    }
    this.read_contacts = function (id, success, error) {
        var data = [];
        data[0] = id;
        this.call("readcontacts", JSON.stringify(data), function (a) {
            if (success)
                success(a);
        }, function (e) {
            if (error)
                error(e);
        });
    }
    this.add_contacts = function (id, contacts, success, error) {
        var data = [];
        data[0] = id;        
        //contacts as [{id:"",alias:""}]
        console.log(contacts);
        data[1] = contacts;
        this.call("addcontacts", JSON.stringify(data), function (a) {
            if (success)
                success(a);
        }, function (e) {
            if (error)
                error(e);
        });
    }
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

