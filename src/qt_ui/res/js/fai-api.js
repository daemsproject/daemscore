var IMAGE_FILE_TYPE = ["image/jpeg", "image/png", "image/svg+xml", "image/tiff", "image/bmp"];
var VIDEO_FILE_TYPE = ["video/mp4"];
var AUDIO_FILE_TYPE = ["audio/mpeg"];
var FAI = "Φ";
var fai = "φ";

var FAI_API = new function () {
    var apiconnected = false;
    var notifyblockfunc;
    var notifytx = {func: "", ids: []};
    var notifypeerfunc;
    var notifyaccountfunc;
    var notifyidfunc;
    var notifyfallbackfunc;
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
//        console.log("notify:" + notifyjson);
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
                    //console.log("notifytx id:" + notifytx.ids[i]);
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
                break;
            case "newID":
                cmd = 'var a=' + notifyidfunc + ';a(data);';
                break;
            case "fallback":
                cmd = 'var a=' + notifyfallbackfunc + ';a(data);';
                break;

        }
        //console.log(cmd);
        eval(cmd);
    }
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
//        console.log("fai-api.call:" + jsreplyjson);
        try {
            jsreply = $.parseJSON(jsreplyjson);
        }
        catch (e) {
            errorfunc(jsreplyjson);
        }
//        console.log("fai-api.call:" + jsreply);
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
//        console.log("fai-api.call:" + jsreply);
        successfunc(jsreply);
    };
    this.icall = function (cmd, datajson) {
        var jsreply;
        var jsreplyjson = jsinterface.jscall(cmd, JSON.stringify(datajson));
        //console.log("fai-api.call:" + jsreplyjson);
        if (jsreplyjson == "null" || !jsreplyjson) {
            return null;
        }
//        console.log(String(jsreplyjson));
        if (jsreplyjson == '"OK"' || jsreplyjson == '"success"' || jsreplyjson == "OK" || jsreplyjson == "success")
            return true;
        try {
            jsreply = $.parseJSON(jsreplyjson);
        }
        catch (e) {
            console.log("icall error:" + cmd + e);
            return false;
        }
        if (jsreply.error) {
            if (jsreply.error.message)
                console.log("icall error:" + cmd + " " + jsreply.error.message);
            else
                console.log("icall error:" + cmd + " " + jsreply.error);
            return false;
        }
        return jsreply;
    };
    this.getAccountID = function () {
        return this.icall("getmainid", []);
    };
    this.getIDs = function (id) {
        var jsreply = this.icall("getidlist", [id]);
        return jsreply.ids;
    };
    this.requestPayment = function (fromIDs, toID, amount, content, feerate, locktime, success, error) {
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
        if (locktime && !isNaN(locktime))
            vout.locktime = locktime;
        if (content)
            vout.content = content;
        if (feerate && (!isNaN(feerate)))
            request.feerate = feerate;
        request.vout[0] = vout;
        data[0] = request;
        console.log(JSON.stringify(data));
        this.call("requestpayment", data, success, error, false);

    }
    this.requestOverride = function (id, txid, feerate, locktime, success, error) {
        var p = {};
        if (locktime && !isNaN(locktime))
            p.locktime = locktime;
        if (feerate && (!isNaN(feerate)))
            p.feerate = feerate;
        console.log(p);
        this.call("requestoverride", [id, txid, p], success, error, false);
    }
    this.createTxByContent = function (ctt, feeRate, toId, deposit, locktime) {
        feeRate = typeof feeRate === "undefined" ? 0 : feeRate;
        locktime = typeof locktime === "undefined" ? 0 : locktime;
        deposit = typeof deposit === "undefined" ? 0 : deposit;

        var accountID = FAI_API.getAccountID();
//        console.log(feeRate);
        var targetID = toId ? toId : (locktime > 0 ? accountID : "");
//        console.log(targetID);
        var f;
        if (toId)
            deposit = locktime = 0;
        this.call("requestpayment2", [accountID, targetID, [ctt.hex], feeRate, deposit, locktime], function (r) {
            console.log('r ' + new Date());
            console.log(r);
            f = r;
        }, function (e) {
            console.log('e ' + new Date());
            console.log(e);
            f = e;
        }, false);
        return f;
    };
    this.createTxByContents = function (ctts, feeRate, toId, deposit, locktime) {
        console.log(ctts);
        locktime = typeof locktime === "undefined" ? 0 : locktime;
        deposit = typeof deposit === "undefined" ? 0 : deposit;
        var accountID = FAI_API.getAccountID();
        var targetID = toId ? toId : (locktime > 0 ? accountID : "");
        var f;
        if (toId) {
            deposit = 0;
            locktime = 0;
        }
        var cttInput = [];
        for (var i in ctts)
            cttInput.push(ctts[i].hex);
        console.log(cttInput);
        console.log(feeRate);
        console.log(deposit);
        console.log(locktime);
        this.call("requestpayment2", [accountID, targetID, cttInput, feeRate, deposit, locktime], function (r) {
            f = r;
        }, function (e) {
            f = e;
        }, false);
        return f;
    };
    this.listtransactions = function (id, success, error, number, offset) {
        var data = [];
        data[0] = id;
        data[1] = (number && isNaN(number)) ? number : 10000;
        data[2] = (offset && isNaN(offset)) ? offset : 0;
        this.call("listtransactions", data, function (result) {
            if (success)
                success(result);
        }, function (e) {
            if (error)
                error(e);
        });

    };
    this.clearCache = function () {
        return this.icall("clearcache", []);
    }
    this.getBalance = function (id, fUseWallet) {
        if (typeof fUseWallet === "undefined")
            fUseWallet = true;
        return this.icall("getbalance", [[id], fUseWallet])
    }
    this.regNotifyBlocks = function (func) {
        this.connectSlots();
        notifyblockfunc = func;
    };
    this.regNotifyTxs = function (func, ids) {
        this.connectSlots();
        notifytx.func = func;
        notifytx.ids = ids;
    };
    this.regNotifyPeers = function (func) {
    }
    this.regNotifyAccount = function (func) {
        this.connectSlots();
        notifyaccountfunc = func;
    }
    this.regNotifyFallback = function (func) {
        this.connectSlots();
        notifyfallbackfunc = func;
    }
    this.regNotifyID = function (func) {
        this.connectSlots();
        notifyidfunc = func;
    }
    this.getInfo = function () {
        return this.icall("getinfo", "")
    };
    this.getLang = function () {
        return this.icall("getlang");
    }
    this.setLang = function (lang) {
        return this.icall("setlang", [lang]);
    }
    this.getBlockCount = function () {
        return JSON.stringify(this.icall("getblockcount", []));
    };
    this.getContentByLink = function (c, f) {
        f = typeof f === "undefined" ? 6 : f;
        return this.icall("getcontentbylink", [c, f]);
    };
//    this.setFollow = function (a) {
//        return this.icall("setfollow", [[a]]);
//    };
//    this.setUnfollow = function (a) {
//        return this.icall("setunfollow", [[a]]);
//    };
//    this.getFollowed = function () {
//        return this.icall("getfollowed", []);
//    };
    this.getHash = function (a) {
        return this.icall("gethash", [a]);
    };
    this.goToCustomPage = function (a) {
        return this.icall("gotocustompage", [a]);
    };

    this.getContents = function (flink, blkc, fAsc) {
        flink = typeof flink !== 'undefined' ? flink : CLink.set(0).toString();
        blkc = typeof blkc !== 'undefined' ? blkc : 10;
        fAsc = typeof fAsc !== 'undefined' ? fAsc : true;
        var wcc = ["CC_FILE_P", "CC_TEXT_P", "CC_TEXT", "CC_LINK_P", "CC_LINK", "CC_PRODUCT_P", "CC_FILE_PACKAGE_P"];
        var wocc = ["CC_DOMAIN_P", "CC_MESSAGE_P", "CC_NULL"];
        var d = [{flink: flink, maxc: 10, maxb: 3000000, withcc: wcc, withoutcc: wocc, cformat: 6, fAsc: fAsc, mincsize: 3, blkc: blkc}];
        console.log(JSON.stringify(d));
        return this.icall("getcontents", d);
    };
    this.getContentsByAddresses = function (nFile, nPos, nRange, ftxCount, frAddrs, toAddrs, fAsc) {
        fAsc = typeof fAsc !== 'undefined' ? fAsc : true;
        frAddrs = typeof frAddrs !== 'undefined' ? frAddrs : [];
        toAddrs = typeof toAddrs !== 'undefined' ? toAddrs : [];
        var wcc = ["CC_FILE_P", "CC_TEXT_P", "CC_TEXT", "CC_LINK_P", "CC_LINK", "CC_PRODUCT_P", "CC_FILE_PACKAGE_P"];
        var wocc = ["CC_DOMAIN_P", "CC_MESSAGE_P", "CC_NULL"];
        var d = {maxc: 20, maxb: 3000000, withcc: wcc, withoutcc: wocc, cformat: 6, fAsc: fAsc, mincsize: 3, frAddrs: frAddrs, toAddrs: toAddrs};
        if (nFile >= 0)
            d.nFile = nFile;
        if (nPos >= 0)
            d.nPos = nPos;
        if (nRange >= 0)
            d.nRange = nRange;
        if (ftxCount >= 0)
            d.ftxCount = ftxCount;
        console.log(JSON.stringify(d));
        return this.icall("getcontentsbyaddresses", [d]);
    };
    this.getProductContentsByAddresses = function (ids) {
        var wcc = ["CC_PRODUCT_P"];
        var d = {maxc: 1000, maxb: 10000000, withcc: wcc, cformat: 7, mincsize: 3, frAddrs: ids};
        console.log(JSON.stringify(d));
        return this.icall("getcontentsbyaddresses", [d]);
    };
    this.getPromotedContents = function (params)
    {

        return this.icall("getpromotedcontents", [params]);
    }
    this.getSalesRecord = function (id, nMax, nOffset) {
        nMax = nMax ? nMax : 1000;
        nOffset = nOffset ? nOffset : 0;
        if (Object.prototype.toString.call(id) != '[object Array]')
            id = [id];
        return this.icall("getsalesrecord", [id, nMax, nOffset]);
    };
    this.getPurchaseRecord = function (id, nMax, nOffset) {
        nMax = nMax ? nMax : 1000;
        nOffset = nOffset ? nOffset : 0;
        if (Object.prototype.toString.call(id) != '[object Array]')
            id = [id];
        return this.icall("getpurchaserecord", [id, nMax, nOffset]);
    };
    this.isContentImage = function (ctt) {
//        console.log(atob(ctt.content[0].content[1].content));
        var imgMIME = ["image/jpeg", "image/png", "image/gif", "image/bmp", "image/tiff"];
        if (ctt.content[0].cc_name !== "CC_FILE_P")
            return false;
        for (var i = 0, t; t = ctt.content[0].content[i]; i++) {
            if (t.cc_name === "CC_FILE_TYPESTRING" && jQuery.inArray(atob(t.content), imgMIME) >= 0) {
                return true;
            }
        }
        return false;
    };
    this.getImages = function (fbh, blkc, fAsc, maxc) {
        fbh = typeof fbh !== 'undefined' ? fbh : 0;
        blkc = typeof blkc !== 'undefined' ? blkc : 10;
        fAsc = typeof fAsc !== 'undefined' ? fAsc : true;
        maxc = typeof maxc !== 'undefined' ? maxc : 10;
        var wocc = ["CC_FILE_PACKAGE_P", "CC_DOMAIN_P", "CC_MESSAGE_P", "CC_FILE_PART_P", "CC_NULL"];
        var wccc = {"cc_name": "CC_FILE_TYPESTRING", "ccontent": IMAGE_FILE_TYPE, "format": "bin"};
        var wcc = ["CC_FILE_P", "CC_P", "CC_FILE_TYPESTRING"];
        var d = [{fbh: fbh, maxc: maxc, maxb: 20000000, withcc: wcc, withccc: wccc, withoutcc: wocc, cformat: 6, fAsc: fAsc, mincsize: 3, blkc: blkc}];
//        console.log(JSON.stringify(d));
        var r = this.icall("getcontents", d);
//        console.log(r);
        return r;
    };
//    this.testGetContents = function (fbh) {
//        var fcc = ["CC_FILE_P", "CC_TEXT_P", "CC_LINK_P", "CC_LINK"];
//        var d = [{"fbh": fbh, "maxc": 20, "maxb": 3000000, "firstcc": fcc, "cformat": 6, "fAsc": false, "mincsize": 3, "blkc": 10}];
//        return this.icall("getcontents", d);
//    };
    this.getMessages = function (idsLocal, idsForeign, directionFilter, fIncludeMempool, fListOnly, startBlock, offset, number, success, error) {
        var d = {};
        if (!idsLocal) {
            if (error)
                error("no local IDs provided");
            return false;
        }
        //never collects message to idslocal itself

        if (Object.prototype.toString.call(idsLocal) != '[object Array]')
            idsLocal = [idsLocal];
        //if idsForeign is provided, only gets messages between local & foreign.
        if (idsForeign)
        {
            if (Object.prototype.toString.call(idsForeign) != '[object Array]')
                idsForeign = [idsForeign];
            d.IDsForeign = idsForeign;
        }
        //direction filter:        0:both in/out        1:incoming only        2:outputs only
        if (directionFilter)
            d.directionFilter = directionFilter;

        if (fListOnly)
            d.fListOnly = fListOnly;
        if (fIncludeMempool)
            d.fincludeMempool = fIncludeMempool;
        if (startBlock)
            d.startBlock = startBlock;
        if (offset)
            d.offset = offset;
        if (number)
            d.count = number;
        var a = this.icall("getmessages", [idsLocal, d]);
        if (a.error) {
            if (error)
                if (a.error.message)
                    error(a.error.message)
                else
                    error(a.error);
            return false;
        }
        if (success)
            success(a);
        return a;
    }
    this.getTxMessages = function (idsLocal, txs, success, error) {
        if (!idsLocal) {
            if (error)
                error("no local IDs provided");
            return false;
        }
        //never collects message to idslocal itself        
        if (Object.prototype.toString.call(idsLocal) != '[object Array]')
            idsLocal = [idsLocal];
        var a = this.icall("gettxmessages", [idsLocal, txs]);
        if (a.error) {
            if (error)
                if (a.error.message)
                    error(a.error.message)
                else
                    error(a.error);
            return false;
        }
        if (success)
            success(a);
        return a;
    }
    this.sendMessage = function (idLocal, idForeign, msg, feeratein, success, error) {
        var feerate = 1000;
        if (feeratein && (!isNaN(feeratein)))
            feerate = feeratein;
        this.call("sendmessage", [idLocal, idForeign, msg, feerate], success, error);
    }
    this.getMiningInfo = function () {
        return this.icall("getmininginfo", [])
    }
    this.setGenerate = function (generate, id, kernels, fnewkey, success, error) {
        fnewkey = typeof fnewkey === "undefined" ? false : fnewkey;
        this.call("setgenerate", [generate, Number(kernels), id, fnewkey], function (a) {
            if (success)
                success(a);
        }, function (e) {
            if (error)
                error(e);
        });
    }
    this.getDomainsByOwner = function (id) {
        if (Object.prototype.toString.call(id) != '[object Array]')
            id = [id];
        return this.icall("getdomainsbyowner", [id]);
    }
    this.getDomainByForward = function (id) {
        return this.icall("getdomainbyforward", [id]);
    };
    this.getDomainsByForward = function (id) {
        if (Object.prototype.toString.call(id) != '[object Array]')
            id = [id];
        return this.icall("getdomainsbyforward", [id]);
    };
    this.getDomainsByTags = function (tags, nMax, finclude100) {
        nMax = nMax ? nMax : 1000;
        finclude100 = finclude100 ? true : false;
        if (Object.prototype.toString.call(tags) != '[object Array]')
            tags = [tags];
        return this.icall("getdomainsbytags", [tags, nMax, finclude100]);
    };
    this.getDomainInfo = function (d) {
        if (Object.prototype.toString.call(d) != '[object Array]')
            d = [d];
        var domain = this.icall("getdomaininfo", [d]);
        return domain[0] ? domain[0] : "";
    }
    this.registerDomain = function (id, domain, lockvalue, locktime, feerate, success, error) {
        this.call("registerdomain", [id, domain, lockvalue, locktime, feerate], function (a) {
            if (success)
                success(a);
        }, function (e) {
            if (error)
                error(e);
        });
    }
    this.renewDomain = function (id, domain, lockvalue, locktime, feerate, success, error) {
        this.call("renewdomain", [id, domain, lockvalue, locktime, feerate], function (a) {
            if (success)
                success(a);
        }, function (e) {
            if (error)
                error(e);
        });
    }
    this.updateDomain = function (id, d, info, feerate, success, error) {
        this.call("updatedomain", [id, d, info, feerate], function (a) {
            if (success)
                success(a);
        }, function (e) {
            if (error)
                error(e);
        });
    }
    this.transferDomain = function (id, domain, idto, feerate, success, error) {
        this.call("transferdomain", [id, domain, idto, feerate], function (a) {
            if (success)
                success(a);
        }, function (e) {
            if (error)
                error(e);
        });
    }
    this.publishProduct = function (id, product, lockvalue, feerate, success, error) {
        if (Object.prototype.toString.call(product) != '[object Array]')
            product = [product];
        this.call("publishproduct", [id, product, lockvalue, feerate], function (a) {
            if (success)
                success(a);
        }, function (e) {
            if (error)
                error(e);
        });
    }
    this.searchProducts = function (params) {
//        console.log(params);
        return this.icall("searchproducts", [params]);
    }
    this.getProductByLink = function (link) {
        //console.log(params);
        return this.icall("getproductbylink", [link]);
    }
    this.buyProducts = function (id, products, feerate, success, error) {
        if (Object.prototype.toString.call(products) != '[object Array]')
            products = [products];
        this.call("buyproduct", [id, products, feerate], function (a) {
            if (success)
                success(a);
        }, function (e) {
            if (error)
                error(e);
        });
    }
    this.publishPackage = function (id, json, feerate, lockvalue, locktime) {
        if (!lockvalue)
            lockvalue = 0;
        if (!locktime)
            locktime = 0;
        return this.icall("publishpackage", [id, json, feerate, lockvalue, locktime])
    }
    this.getUnspent = function (id) {
        if (Object.prototype.toString.call(id) != '[object Array]')
            id = [id];
        return this.icall("listunspent2", [id]);
    }
    //this.read_contacts = function (id) {return this.icall("readcontacts", [id]);};
    //this.add_contacts = function (id, contacts) {return this.icall("addcontacts", [id, contact])}
    this.setConf = function (app, idlocal, idforeign, conf, value) {
        if (!value)
            value = "";
        return this.icall("setconf", [app, idlocal, idforeign, conf, String(value)]);
    }
    this.getConf = function (app, idlocal, idforeign, conf) {
        var result = this.icall("getconf", [app, idlocal, idforeign, conf]);
        if (result.error)
            return "";
        return $.parseJSON(result);
    }
    this.writeFile = function (app, path, filename, filestring) {
        return this.icall("writefile", [app, path, filename, filestring]);
    }
    this.writeFile2 = function (filename, filestring) {
        return this.icall("writefile2", [filename, filestring]);
    }
    this.readFile = function (app, path, filename) {
        var result = this.icall("readfile", [app, path, filename]);
        if (result.error)
            return "";
        return result;
    }
    this.getSettings = function () {
        return this.icall("getsettings");
    }
    this.updateSettings = function (type1, type2, value) {
        return this.icall("updatesettings", [String(type1), String(type2), String(value)]);
    }
    this.signMessage = function (id, msg4sig) {
        return this.icall("signmessage", [id, msg4sig]);
    };
    this.getFeeRate = function (point) {
        if (typeof point === "undefined")
            return this.icall("getfeerate", []);
        else
            return this.icall("getfeerate", [point]);
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

    this.getMatureTime = function (locktime) {
        return this.icall("getmaturetime", [locktime]);
    }
    this.encryptMessages = function (idLocal, msgArr) {
        return this.icall("encryptmessages", [idLocal, msgArr]);
    }
    this.decryptMessages = function (idLocal, msgArr, success, error) {
        return this.call("encryptmessages", [idLocal, msgArr, false], success, error);
    }
    this.areIDsEqual = function (id1, id2) {
        return this.icall("comparebase32", [id1, id2]) == 0
    };
    this.createPContent = function (hex) {
        return this.icall("encodecontentunit", ["CC_P", hex, 0]);
    };
    this.createContentC = function (t, c) {  // deprecated
        console.log("t " + t);
        console.log("c " + JSON.stringify(c));
        switch (t)
        {
            case "text/plain":
                var u1 = this.icall("encodecontentunit", ["CC_TEXT", c.data, 2]);
                var u2 = this.icall("encodecontentunit", ["CC_TEXT_ENCODING_UTF8", "", 2]);
                var s = u1.hex + u2.hex;
                var r = this.icall("encodecontentunit", ["CC_TEXT_P", s, 0]);
                break;
            case "image/png":
            default:
                var u1 = this.icall("encodecontentunit", ["CC_NAME", "t", 2]);
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
    this.createHexContentByJson = function (ctt) {
        var hex = "";
        if (typeof ctt.content === "object") {
            var tmp0 = "";
            console.log(ctt);
            for (var i in ctt.content) {
                console.log(i);
                var tmp = this.createHexContentByJson(ctt.content[i]);
                console.log(tmp);
                tmp0 += tmp;
                console.log(tmp0);
            }
            console.log(ctt);
            if (typeof ctt.cc_name !== "undefined") {
                var tmp1 = this.icall("encodecontentunit", [ctt.cc_name, tmp0, 0]);
                hex = tmp1.hex;
            } else
                hex = tmp0;
        }
        else if ((typeof ctt.content === "undefined"))
            console.log(ctt);
        else {
            console.log(ctt);
            var tmp = this.icall("encodecontentunit", [ctt.cc_name, ctt.content, ctt.encoding]);
            console.log(tmp);
            hex = tmp.hex;
        }
        return hex;
    };
    this.createTextContent = function (t) {
        return this.icall("encodecontentunit", ["CC_TEXT", t, 1]);
    };
    this.createTagContent = function (t) {
        if (t === "CC_TAG_EROTIC" || t.toLowerCase() == "erotic" || t.toLowerCase() == "sex" || t.toLowerCase() == "porn" || t.toLowerCase() == "x-rated")
            return this.icall("encodecontentunit", ["CC_TAG_EROTIC", "", 1]);
        else if (t === "CC_TAG_HORROR" || t.toLowerCase() == "horror" || t.toLowerCase() == "terror")
            return this.icall("encodecontentunit", ["CC_TAG_HORROR", "", 1]);
        else if (t === "CC_TAG_POLITICS" || t.toLowerCase() == "politics" || t.toLowerCase() == "political")
            return this.icall("encodecontentunit", ["CC_TAG_POLITICS", "", 1]);
        return this.icall("encodecontentunit", ["CC_TAG", t, 1]);
    };
    this.createLinkContent = function (l) {
        l.linktype = typeof l.linktype === "undefined" ? "UNKNOWN" : l.linktype;
        if (l.linktype === "BLOCKCHAIN") {
            var vh = this.icall("encodevarint", [l.nHeight]);
            var vt = this.icall("encodevarint", [l.nTx]);
            var vo = this.icall("encodevarint", [l.nVout]);
            var u1 = this.icall("encodecontentunit", ["CC_LINK_TYPE_BLOCKCHAIN"]);
            var u2 = this.icall("encodecontentunit", ["CC_LINK", String(vh) + String(vt) + String(vo), 0]);
            var f = "";
            if (typeof l.linkname !== "undefined") {
                if (l.linkname !== "") {
                    var u3 = this.icall("encodecontentunit", ["CC_NAME", l.linkname, 1]);
                    f = u1.hex + u2.hex + u3.hex;
                }
            }
            f = f === "" ? u1.hex + u2.hex : f;
            var r = this.icall("encodecontentunit", ["CC_LINK_P", f, 0]);
        } else if (l.linktype === "HTTP") {
            var u1 = this.icall("encodecontentunit", ["CC_LINK_TYPE_HTTP"]);
            var u2 = this.icall("encodecontentunit", ["CC_LINK", l.linkstr, 1]);
            var f = "";
            if (l.linkname !== "") {
                var u3 = this.icall("encodecontentunit", ["CC_NAME", l.linkname, 1]);
                f = u1.hex + u2.hex + u3.hex;
            }
            f = f === "" ? u1.hex + u2.hex : f;
            var r = this.icall("encodecontentunit", ["CC_LINK_P", f, 0]);
        }
        return r;
    };
    this.createFileContent = function (f) {
        var u1 = this.icall("encodecontentunit", ["CC_NAME", f.name, 1]);
        var u2 = this.icall("encodecontentunit", ["CC_FILE_TYPESTRING", f.type, 1]);
        var u3 = this.icall("encodecontentunit", ["CC_FILE", f.data, 2]);
        var u = u1.hex + u2.hex + u3.hex;
        var s = this.icall("encodecontentunit", ["CC_FILE_P", u, 0]);
        return s;
    }
    this.createFilePartContent = function (fp) {
        var u1 = this.icall("encodecontentunit", ["CC_NAME", fp.name, 1]);
        var u2 = this.icall("encodecontentunit", ["CC_FILE_TYPESTRING", fp.type, 1]);
        var u3 = this.icall("encodecontentunit", ["CC_FILE_PART", fp.data, 2]);
        var n = this.icall("encodevarint", [fp.nPart]);
        var u4 = this.icall("encodecontentunit", ["CC_FILE_PART_NUMBER", String(n), 0]);
        var u = u1.hex + u2.hex + u3.hex + u4.hex;
        var s = this.icall("encodecontentunit", ["CC_FILE_PART_P", u, 0]);
        return s;
    }
    this.b32CheckEncode = function (hex) {
        return this.icall("encodebase32check", [hex]);
    }
    this.b32CheckDecode = function (b32) {
        return this.icall("decodebase32check", [b32]);
    }
    this.verifyMessage = function (id, sig, msg4sig) {
        return this.icall("verifymessage", [id, sig, msg4sig]);
    }

};