var satoshi = 1000000;
var root = "/";
var resource = "../";
var war_checksum;
var min = true;
var isExtension = false;
var APP_VERSION = "1.0";
var APP_NAME = "javascript_web";
var IMPORTED_APP_NAME = "external";
var IMPORTED_APP_VERSION = "0";
var gParam = {}; // global params
function stripHTML(a) {
    return $.trim($("<div>" + a.replace(/(<([^>]+)>)/ig, "") + "</div>").text())
}
$.fn.center = function () {
    scrollTo(0, 0);
    this.css("top", parseInt(Math.max(($(window).height() / 2) - (this.height() / 2), 0)) + "px");
    this.css("left", parseInt(Math.max(($(window).width() / 2) - (this.width() / 2), 0)) + "px");
    return this;
};

//if (!window.console) {
//    var names = ["log", "debug", "info", "warn", "error", "assert", "dir", "dirxml", "group", "groupEnd", "time", "timeEnd", "count", "trace", "profile", "profileEnd"];
//    window.console = {};
//    for (var i = 0; i < names.length; ++i) {
//        window.console[names[i]] = function () {
//        }
//    }
//}


Date.prototype.sameDayAs = function (a) {
    return((this.getFullYear() == a.getFullYear()) && (this.getMonth() == a.getMonth()) && (this.getDate() == a.getDate()))
};
function padStr(a) {
    return(a < 10) ? "0" + a : "" + a
}
function convert(a, b) {
    return(a / b).toFixed(2).toString().replace(/(\d)(?=(\d\d\d)+(?!\d))/g, "$1,")
}
function sShift(a) {
    return(satoshi / a.conversion).toString().length - 1
}

function updateQueryString(b, d, a) {
    if (!a) {
        a = window.location.href
    }
    var c = new RegExp("([?|&])" + b + "=.*?(&|#|$)(.*)", "gi");
    if (c.test(a)) {
        if (typeof d !== "undefined" && d !== null) {
            return a.replace(c, "$1" + b + "=" + d + "$2$3")
        } else {
            return a.replace(c, "$1$3").replace(/(&|\?)$/, "")
        }
    } else {
        if (typeof d !== "undefined" && d !== null) {
            var f = a.indexOf("?") !== -1 ? "&" : "?", e = a.split("#");
            a = e[0] + f + b + "=" + d;
            if (e[1]) {
                a += "#" + e[1]
            }
            return a
        } else {
            return a
        }
    }
}
function loadScript(j, h, a) {
    var f = false;
    $("script").each(function () {
        var e = $(this).attr("src");
        if (e && e.replace(/^.*[\\\/]/, "").indexOf(j) == 0) {
            h();
            f = true;
            return false
        }
    });
    if (f) {
        return
    }
    console.log("Load " + j);
    var d = false;
    var c = document.createElement("script");
    c.type = "text/javascript";
    c.async = true;
    c.src = resource + j + (min ? ".min.js" : ".js") + "?" + war_checksum;
    try {
        c.addEventListener("error", function (k) {
            d = true;
            if (a) {
                a("Error Loading Script. Are You Offline?")
            }
        }, false);
        c.addEventListener("load", function (k) {
            if (!d) {
                h()
            }
        }, false)
    } catch (g) {
        setTimeout(function () {
            if (!d) {
                h()
            }
        }, 10000)
    }
    var b = document.getElementsByTagName("head")[0];
    b.appendChild(c)
}
function createImgHtml(imgB64Data, clink) {
    var html = '<a ><img';
    if (clink)
        html += ' id="' + clink;
    html += '" src="data:image/jpg;base64,' + imgB64Data + '" class="brimg"/></a>';
    return html;
}
;
function showID(id) {
    return id.substr(0, 10) + "......" + id.substr(-2);
}
;
function IsLevel2Domain(d) {
    if (!IsValidDomain)
        return false;
    return (d.substring(d.indexOf(".") + 1).indexOf(".") != -1);
}
function IsValidDomain(d) {
    if (d.indexOf("@") > -1 || d.indexOf(":") > -1 || d.indexOf(" ") > -1 || d.indexOf(".") == 0 || d.indexOf("..") > -1)
        return false;
    if (d.indexOf("-") == 0)
        return false;
    if (d.indexOf("--") > -1)
        return false;
    if (d.indexOf("-.") > -1 || d.indexOf(".-") > -1)
        return false;
    var pszBaseDomain = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-";

    for (var i = 0; i < d.length; i++)
    {
        var f = false;
        for (var j = 0; j < pszBaseDomain.length; j++) {
            if (pszBaseDomain.substr(j, 1) === d.substr(i, 1)) {
                f = true;
                break;
            }
        }
        if (f === false)
            return false;
    }
    if (d.substring(d.length - 2) == ".f" || d.substring(d.length - 4) == ".fai")
        return true;
    return false;
}
function GetLevel1Domain(d) {
    if (!IsValidDomain(d))
        return "";
    var x = d;
    while (IsLevel2Domain(x))
        x = x.substring(x.indexOf(".") + 1);
    return x;
}

function getB64DataFromLink(clink) {
    var cj = (FAI_API.getContentByLink(clink));
    r = this.getFileContentFrJson(cj);
    return r;//rcreateImgHtml(r);
}
;
function parseTx(tx, IDs) {
    tx.amount = 0;
    if (!tx.blockheight)
        tx.confirmations = 0;
    if (tx.iscoinbase) {
        tx.category = "minted";
        for (var j in tx.vout)
            tx.amount += Number(tx.vout[j].value);
        tx.address = tx.vout[0].scriptPubKey.address;
    } else {
        var fromLocal = false;
        var fromForeign = false;
        var toForeign = false;
        var fromLocalAddress;
        var fromForeignAddress;
        var toForeignAddress;
        var toLocalAddress;
        for (var j in tx.vin) {
            var inid = tx.vin[j].scriptPubKey.address;
            var isOwnID = false;
            for (var k in IDs)
                if (IDs[k] == inid)
                    isOwnID = true;
            if (isOwnID) {
                fromLocal = true;
                tx.amount -= Number(tx.vin[j].value);
                fromLocalAddress = inid;
            } else {
                fromForeign = true;
                fromForeignAddress = inid;
            }
        }
        for (var j in tx.vout) {

            var outid = TR("Publishing content");
            if (tx.vout[j].scriptPubKey.address)
                outid = tx.vout[j].scriptPubKey.address;
            var isOwnID = false;
            for (var k in IDs)
                if (IDs[k] == outid)
                    isOwnID = true;
            if (isOwnID) {
                tx.amount += Number(tx.vout[j].value);
                toLocalAddress = outid;
            } else {
                toForeign = true;
                toForeignAddress = outid;
            }
        }
        if (fromLocal && fromForeign) {
            if (tx.amount > 0) {
                tx.category = "receive";
                tx.address = fromForeignAddress;
            }
            else {
                tx.category = "send";
                tx.address = toLocalAddress;
            }
        } else if (fromForeign) {
            tx.category = "receive";
            tx.address = fromForeignAddress;
        } else if (toForeign) {
            tx.category = "send";
            tx.address = toForeignAddress;
        } else {
            tx.category = "toSelf";
            tx.address = toLocalAddress;
        }
    }
    tx.amount = tx.amount.toFixed(6);
    return tx;
}
function getStrLinkType(str)
{
    if (CLink.setString(str).isValid())
        return "link_blockchain";
    if (IsValidDomain(str))
        return "domain";
    var b32d = FAI_API.b32CheckDecode(str);
    if (b32d && !b32d.error)
        return "id";
    return "other";
}


function resetC2() {
    if ($("#shdr").length > 0) {
        var c1w = $("body").width() - $(".column1").width() - 150;
        $(".column2").css({width: c1w});
    } else {
        var c1w = $(window).width() * 0.95;
        $(".column2").css({left: 0});
        $(".column2").css({width: c1w});
    }
    var height = $(window).height() - 50;
    $(".column2").css({height: height});
}
function prepareStdTpl() {
    $(".id").find("a.text").unbind().click(function () {
        CBrowser.toggleIdOpt($(this).parent());
    });
    $(".cmt").find("a.shrt").unbind().click(function () {
        CBrowser.toggleCmt($(this).parent());
    });
    $(".id-follow-btn").unbind().click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var domain = $(this).parent().parent().find(".text").attr("domain");
        var id2fll = domain ? domain : id;
        var feedback = domain ? CBrowser.setFollow(id2fll, "domain") : CBrowser.setFollow(id2fll, "id");
        for (var k in feedback) {
            if (feedback[k] == id2fll) {
                CPage.showNotice(TR('Successfully followed ') + id2fll);
                return;
            }
        }
        CPage.showNotice("Failed");
    });
    $(".id-unfollow-btn").unbind().click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var domain = $(this).parent().parent().find(".text").attr("domain");
        var id2fll = domain ? domain : id;
        var feedback = domain ? CBrowser.setUnfollow(id2fll, "domain") : CBrowser.setUnfollow(id2fll, "id");
        if (feedback === true) {
            CPage.showNotice(TR('Successfully unfollowed ') + id2fll);
            return;
        }
        CPage.showNotice("Failed");
    });
    $(".id-copyid-btn").unbind().click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        if (typeof id === "undefined")
            id = $(this).parent().parent().parent().parent().find(".navi-name").attr("fullid");
        CUtil.copyToClipboard(id);
    });
    $(".id-homepage-btn").unbind().click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var domain = $(this).parent().parent().find(".text").attr("domain");
        var url = domain ? domain : "fai:browser/?id=" + id;
        FAI_API.goToCustomPage(url);
    });
    $(".id-chat-btn").unbind().click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var url = "fai:messenger/?chatto=" + id;
        FAI_API.goToCustomPage(url);
    });
    $(".id-share-btn").unbind().click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var domain = $(this).parent().parent().find(".text").attr("domain");
        var id2share = domain ? domain : id;
        CPublisher.clearWithAlert("shareid");
        CPublisher.shareId(id2share);
    });
    $(".ctt-link-btn").unbind().click(function () {
        var link = $(this).parent().parent().find(".linkspan").attr("clink");
        CUtil.copyToClipboard(link);
    });
    $(".ctt-share-btn").unbind().click(function () {
        var link = $(this).parent().parent().find(".linkspan").attr("clink");
        CPublisher.clearWithAlert("sharectt");
        CPublisher.shareLink(link);
    });

    $(".ctt-cmt-btn").unbind().click(function () {
        var link = $(this).parent().parent().find(".linkspan").attr("clink");
        var id = $(this).parent().parent().parent().parent().parent().find(".id").find(".text").attr("fullid");
        CPublisher.clearWithAlert("cmt");
        CPublisher.commentLink(link, id, "cmt");
    });
    $(".ctt-tip-btn").unbind().click(function () {
        var link = $(this).parent().parent().find(".linkspan").attr("clink");
        var id = $(this).parent().parent().parent().parent().parent().find(".id").find(".text").attr("fullid");
        CPublisher.clearWithAlert("tip");
        CPublisher.commentLink(link, id, "tip");
    });
    $(".prd-pchs-btn").unbind().click(function () {
        var link = $(this).parent().parent().find(".linkspan").attr("clink");
        var url = "fai:shop/?buy=" + link;
        FAI_API.goToCustomPage(url);
    });
}

var CLink = new function () {
    var CLink = this;
    this.nHeight = -1;
    this.nTx = -1;
    this.nVout = -1;
    this.linkname;
    this.linktype = "";
    this.setString = function (str, ln) {
        this.linkname = (typeof ln === 'undefined') ? "" : ln;
        this.nHeight = -1;
        this.nTx = -1;
        this.nVout = -1;
        this.linktype = "";
        if (typeof str === 'undefined')
            return this;
        var pc = str.indexOf(":");
        var head = pc > 0 ? str.substring(0, pc) : "";
        if (pc >= 0)
            str = str.substring(pc + 1);
        var pfd = str.indexOf(".");

        var psd = str.indexOf(".", pfd + 1);
        if (psd >= 0) {
            var ptd = str.indexOf(".", psd + 1);
            if (ptd >= 0)
                return this;
            if (!isNaN(parseInt(str.substring(0, pfd))))
                this.nHeight = parseInt(str.substring(0, pfd));
            if (!isNaN(parseInt(str.substring(pfd + 1))))
                this.nTx = parseInt(str.substring(pfd + 1, psd));
            if (!isNaN(parseInt(str.substring(psd + 1))))
                this.nVout = parseInt(str.substring(psd + 1));
        } else {
            if (!isNaN(parseInt(str.substring(0, pfd))))
                this.nHeight = parseInt(str.substring(0, pfd));
            if (!isNaN(parseInt(str.substring(pfd + 1)))) {
                this.nTx = parseInt(str.substring(pfd + 1));
                this.nVout = 0;
            }
        }
        if (this.isValid())
            this.linktype = "BLOCKCHAIN";
        //console.log("pfd " + pfd + "  psd " + psd);
        if (pfd >= 0 && psd < 0) {
            var dext = str.substring(pfd + 1).toLowerCase();
            if (dext === "f" || dext === "fai")
                this.linktype = "DOMAIN";
        }
        return this;
    };
    this.set = function (nHeight, nTx, nVout) {
        if (typeof nHeight === "undefined")
            return this;
        if (typeof nHeight === "string")
            return this.setString(nHeight);
        if (typeof nHeight !== "number")
            return this;
        if (nHeight < 0) {
            this.nHeight = -1;
            this.nTx = -1;
            this.nVout = -1;
        } else {
            this.nHeight = nHeight;
            this.nTx = typeof nTx === "undefined" ? 0 : nTx;
            this.nVout = typeof nVout === "undefined" ? 0 : nVout;
        }
        return this;
    };
    this.toString = function () {
        return  (this.nHeight >= 0 && this.nTx >= 0) ?
                (this.nVout > 0 ? "fai:" + this.nHeight + "." + this.nTx + "." + this.nVout : "fai:" + this.nHeight + "." + this.nTx) : "";
    };
    this.toHtmlId = function () {
        return  this.nHeight >= 0 && this.nTx >= 0 ?
                (this.nVout > 0 ? this.nHeight + "_" + this.nTx + "_" + this.nVout : this.nHeight + "_" + this.nTx) : "";
    };
    this.isValid = function () {
        return  this.nHeight >= 0 && this.nTx >= 0;
    };
    this.isEmpty = function () {
        return isNaN(this.nHeight) || isNaN(this.nTx);
    };
    this.cmp = function (l1, l2) { // compare, invalid input link return false, l1 > l2 return 1, equal return 0, l1 < l2 return -1
        if (!this.set(l1).isValid())
            return false;
        var h1 = this.nHeight;
        var t1 = this.nTx;
        var o1 = this.nVout;
        if (!this.set(l2).isValid())
            return false;
        var h2 = this.nHeight;
        var t2 = this.nTx;
        var o2 = this.nVout;
        if (h1 != h2)
            return h1 > h2 ? 1 : -1;
        if (t1 != t2)
            return t1 > t2 ? 1 : -1;
        if (o1 != o2)
            return o1 > o2 ? 1 : -1;
        return 0;
    };
    this.nVoutPP = function () {
        this.nVout = this.nVout + 1;
        return this;
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
    this.escapeHtml = function (h, br) {
        br = typeof br === "undefined" ? true : br;
        var str = $("<div>").text(h).html();
        str = br ? str.replace(/(?:\r\n|\r|\n)/g, '<br />') : str;
        return str;
    };
    this.copyToClipboard = function (text) {
        window.prompt(TR("Copy to clipboard: Ctrl+C (Cmd+C for mac), Enter"), text);
    };
    this.getGet = function (val) {
        var result = null;
        var tmp = [];
        var items = location.search.substr(1).split("&");
        for (var index = 0; index < items.length; index++) {
            tmp = items[index].split("=");
            if (tmp[0] === val)
                result = decodeURIComponent(tmp[1]);
        }
        return result;
    };
    this.setGet = function (idx, val, str) {
        str = typeof str === "undefined" ? window.location.href : str;
        var p = str.split("?");
        if (p.length === 1)
            return str + "?" + idx + "=" + val;
        else if (p.length > 2)
            return false;
        var r;
        var items = p[1].split("&");
        var fReplace = false;
        for (var i in items) {
            var tmp = items[i].split("=");
            if (tmp.length < 2)
                items.splice(i);
            if (tmp[0] === idx) {
                items[i] = idx + "=" + val;
                fReplace = true;
            }
        }
        if (!fReplace)
            items.push(idx + "=" + val);
        r = p[0] + "?" + items.join("&");
        return r;
    };
    this.isLinkHttp = function (lstr) {
        return lstr.substr(0, 4).toLowerCase() === "http";
    };
    this.isLinkHttps = function (lstr) {
        return lstr.substr(0, 5).toLowerCase() === "https";
    };
    this.isLinkFai = function (lstr) {
        return lstr.substr(0, 3).toLowerCase() === "fai";
    };
    this.isLinkBlockChain = function (lstr) {
        var link = CLink.setString(lstr);
        return link.isValid();
    };
    this.cleanNullElem = function (a) { // input of this function must be array
        if (!Array.isArray(a))
            return [];
        var r = [];
        for (var i in a) {
            if (a[i] !== null)
                r.push(a[i]);
        }
        return r;
    };
    this.getBalanceLevel = function (balance) {
        var l = 0;
        var b;
        if (typeof balance === "undefined")
            return false;
        else if (typeof balance === "number")
            b = balance;
        else {
            if (typeof balance.balance_total === "undefined")
                return l;
            else
                b = balance.balance_total;
        }
        b += 0.1; // correct the error for large number division
        l = Math.floor(((Math.log(b) / Math.log(10)) * 2 + 1));
        return l;
    };
    this.initGParam = function (balance) {
        gParam = {};
        gParam.accountID = FAI_API.getAccountID();
        if (typeof balance === "undefined")
            gParam.balance = FAI_API.getBalance(gParam.accountID, false).balance;
        else
            gParam.balance = balance;
        gParam.domain = typeof gParam.accountID === "undefined" ? [] : FAI_API.getDomainsByForward(gParam.accountID);
        gParam.domain = gParam.domain.length > 0 ? gParam.domain[0] : null;
        if (gParam.domain) {
            if (gParam.domain.icon) {
                var ctt = FAI_API.getContentByLink(gParam.domain.icon);
                var cttP = CUtil.parseCtt(ctt);
                if (cttP.fdata) {
                    gParam.icon = {};
                    gParam.icon.data = cttP.fdata;
                    gParam.icon.type = cttP.ftype;
                    gParam.icon.name = cttP.fname;
                }

            } else
                gParam.icon = null;
        }
    };
    this.getShortPId = function (fullId) {
        return typeof fullId === "undefined" ? "" : fullId.substr(0, 10) + "..." + fullId.substr(fullId.length - 2);
    };
    this.getLongPId = function (fullId) {
        return typeof fullId === "undefined" ? "" : fullId.substr(0, 25) + "..." + fullId.substr(fullId.length - 2);
    };
    this.parseCtt = function (ctt) {
        var r = {};
        r.text = null;
        r.fdata = null;
        r.ldata = null;
        r.ltext = null;
        r.ltype = null;
        r.selflink = typeof ctt.link === "undefined" ? null : ctt.link;
        r.filepackage = null;
        if (typeof ctt.cc_name === "undefined") {
            var pcc = ctt.content[0].cc_name;
            var c = ctt.content[0];
        } else {
            var pcc = ctt.cc_name;
            var c = ctt;
        }
        if (pcc === "CC_TEXT") {
            r.text = CUtil.escapeHtml(base64.decode(c.content));
        } else if (pcc === "CC_LINK") {
            r.ldata = c.link;
            r.ltext = CUtil.escapeHtml(base64.decode(c.content));
        } else if (pcc === "CC_LINK_TYPE_BLOCKCHAIN") {
            r.ltype = "BLOCKCHAIN";
        } else if (pcc === "CC_LINK_TYPE_HTTP") {
            r.ltype = "HTTP";
        } else if (pcc === "CC_LINK_TYPE_HTTPS") {
            r.ltype = "HTTPS";
        } else if (pcc === "CC_LINK_TYPE_DOMAIN") {
            r.ltype = "DOMAIN";
        } else if (pcc === "CC_NAME") {
            r.name = base64.decode(cCtt);
        } else if (pcc === "CC_FILE_TYPESTRING") {
            r.ftype = base64.decode(cCtt);
        } else if (pcc === "CC_FILE") {
            r.fdata = cCtt;
        } else if (pcc === "CC_P" || pcc === "CC_FILE_P" || pcc === "CC_TEXT_P" || pcc === "CC_LINK_P") {
            for (var i in c.content) {
                var icc = c.content[i].cc_name;
                var cCtt = c.content[i].content;
                if (icc === "CC_FILE_P" || icc === "CC_TEXT_P" || icc === "CC_LINK_P") {
                    var tmp = CUtil.parseCtt(c.content[i]);
                    if (icc === "CC_FILE_P") {
                        r.fname = tmp.name;
                        r.fdata = tmp.fdata;
                        r.ftype = tmp.ftype;
                    } else if (icc === "CC_TEXT_P") {
                        r.text = tmp.text;
                    } else if (icc === "CC_LINK_P") {
                        r.lname = tmp.name;
                        r.ldata = tmp.ldata;
                        r.ltype = tmp.ltype;
                        r.ltext = tmp.ltext;
                    }
                } else if (icc === "CC_TEXT") {
                    r.text = CUtil.escapeHtml(base64.decode(cCtt));
                } else if (icc === "CC_LINK") {
                    r.ldata = c.content[i].link;
                    r.ltext = CUtil.escapeHtml(base64.decode(cCtt));
                } else if (icc === "CC_LINK_TYPE_BLOCKCHAIN") {
                    r.ltype = "BLOCKCHAIN";
                } else if (icc === "CC_LINK_TYPE_HTTP") {
                    r.ltype = "HTTP";
                } else if (icc === "CC_LINK_TYPE_HTTPS") {
                    r.ltype = "HTTPS";
                } else if (icc === "CC_LINK_TYPE_DOMAIN") {
                    r.ltype = "DOMAIN";
                } else if (icc === "CC_NAME") {
                    r.name = base64.decode(cCtt);
                    if (pcc === "CC_LINK_P")
                        r.lname = r.name;
                    else if (pcc === "CC_FILE_P")
                        r.fname = r.name;
                } else if (icc === "CC_FILE_TYPESTRING") {
                    r.ftype = base64.decode(cCtt);
                } else if (icc === "CC_FILE") {
                    r.fdata = cCtt;
                }
            }
        } else if (pcc === "CC_FILE_PACKAGE_P")
            r.filepackage = true;
        if (!r.lname)
            r.lname = r.ldata;
        return r;
    };
    this.hasChild = function (o) {
        for (var i in o) {
            if (typeof o[i] === "object")
                return true;
        }
        return false;
    }
    this.formatTimeLength = function (time) {
        var str;
        if (time < 60) {
            str = time.toFixed(3);
            str += " " + TR('seconds');
        }
        else if (time < 60 * 60) {
            str = (time / 60).toFixed(3);
            str += " " + TR('minutes');
        }
        else if (time < 60 * 60 * 24) {
            str = (time / 3600).toFixed(3);
            str += " " + TR('hours');
        }
        else {
            str = (time / (3600 * 24)).toFixed(3);
            str += " " + TR('days');
        }
        return str;
    };
    this.dateToString = function (a) {
        if (a.sameDayAs(new Date())) {
            return TR('Today') + " " + padStr(a.getHours()) + ":" + padStr(a.getMinutes()) + ":" + padStr(a.getSeconds())
        } else {
            return padStr(a.getFullYear()) + "-" + padStr(1 + a.getMonth()) + "-" + padStr(a.getDate()) + " " + padStr(a.getHours()) + ":" + padStr(a.getMinutes()) + ":" + padStr(a.getSeconds())
        }
    }
    this.dateToShortString = function (a) {
        if (a.sameDayAs(new Date())) {
            return TR('Today') + " " + padStr(a.getHours()) + ":" + padStr(a.getMinutes()) + ":" + padStr(a.getSeconds())
        } else {
            return padStr(a.getFullYear()) + "-" + padStr(1 + a.getMonth()) + "-" + padStr(a.getDate());
        }
    };
    this.isIdDev = function (id) {
        return id === "N7IWEOEBHWMBUM3GZD3GXY2LEDTKDSP2HAL5QWCK";
    };
    this.isProd = function (ctt) {
        if (!ctt.content)
            return false;
        if (ctt.content.length <= 0)
            return false;
        return (ctt.content[0].cc_name === "CC_PRODUCT_P");
    };
    this.parseProd = function (ctt) {
        if (!ctt.link)
            return false;
        var prod = FAI_API.getProductByLink(ctt.link);
        return prod;
    };
    this.isArray = function (i) {
        return  (Object.prototype.toString.call(i) === "[object Array]");
    }
};

var CPage = new function () {
    this.prepareNotice = function (page) {
        var nDiv = $("#main-notices-container-tpl").clone(true, true);
        nDiv.attr("id", "main-notices-container").removeClass("hide");
        switch (page) {
            case "homepage":
            case "link":
            case "msgr":
                $(".column2").prepend(nDiv);
                break;
            case "browser":
                $("#shdr").prepend(nDiv);
                break;
            case "wallet":
            case "shop":
            case "miner":
            case "tools":
            case "domain":
            default:
                $(".main-container").prepend(nDiv);
                break;
        }
    };
    this.showNotice = function (n, t, s, a) { // t: type n/e/w for normal/error/warning
        t = typeof t !== 'undefined' ? t : "n";
        s = typeof s !== 'undefined' ? s : 5;
        a = typeof a !== 'undefined' ? a : 1;
        $("#notices").removeClass("error").removeClass("warning");
        switch (t) {
            case "e":
                $("#notices").addClass("error");
                break;
            case "w":
                $("#notices").addClass("warning");
                break;
            case "n":
            default:
                break;
        }
        $("#notices").html(n).show();
        $("#notices").delay(s * 1000).hide(a * 1000);
    };
    this.prepareHeader = function (fShowId) {
        fShowId = typeof fShowId === "undefined" ? false : fShowId;
        var ddiv = $("#header-info-tpl").clone(true, true);
        ddiv.find(".navi-name").html(gParam.accountID).attr("fullid", gParam.accountID);
        if (fShowId)
            ddiv.find(".navi-name").parent().removeClass("hide");
        $("#navi-bar").find(".container").append(ddiv.children());
        $('#cblc').html(FAI_API.getBlockCount());
    };
    this.createImgHtml = function (ftype, fdata) {
        if (typeof ftype === "undefined")
            return false;
        if (!ftype)
            return false;
        if ($.inArray(ftype, CONTENT_FILE_TYPE.image) < 0)
            return false;
        var idiv = $('<img/>', {
            type: ftype,
            src: this.createImgSrc(ftype, fdata),
        });
        return idiv;
    };
    this.createImgSrc = function (type, b64) {
        return   "data:" + type + ";base64," + b64;
    };
    this.getBalanceHtml = function (bl, number) {
        number = typeof number === "undefined" ? false : number;
        if (bl == 0)
            return "";
        var ml = Math.min(Math.floor((bl - 1) / 4), 4);
        var sl = bl - ml * 4;
        var p = $("<div />");
        var div = $("<div />");
        div.addClass("icon20");
        switch (ml) {
            case 0:
                div.addClass("silvercoin");
                break;
            case 1:
                div.addClass("goldcoin");
                break;
            case 2:
                div.addClass("goldbar");
                break;
            case 3:
                div.addClass("diamond");
                break;
            case 4:
                div.addClass("crown");
                break;
            default:
                return "";
        }
        for (var i = 0; i < sl; i++)
            p.append(div.clone(true, true));
        if (number)
            p.append($("<span />").html(TR("Deposit: ") + fai + number).css("margin", 20).addClass("grey_a"));
        return p;
    };
    this.updateBalance = function (b) {
        gParam.accountID = FAI_API.getAccountID();
        if (typeof b !== "undefined")
            gParam.balance = b;
        else
            gParam.balance = FAI_API.getBalance(gParam.accountID, false).balance;
        var bl = CUtil.getBalanceLevel(gParam.balance);
        if (!bl)
            return;
        $("#blclvl").html(this.getBalanceHtml(bl));
        $('#balance').html(gParam.balance.balance_total);
    };
    this.updateCblc = function () {
        $('#cblc').html(FAI_API.getBlockCount());
    };
    this.prepareProdDiv = function () {
        var ddiv = $("#prod-tpl").clone(true, true).removeAttr("id").removeClass("hide");
        var pdiv = $("#poster-tpl").clone(true, true).removeAttr("id");
        var cdiv = $("#cmt-tpl").clone(true, true).removeAttr("id");
        var bdiv = $("#buy-btn-tpl").clone(true, true).removeAttr("id");
        cdiv.find(".cmt").append(bdiv.children());
        ddiv.find(".container").prepend(pdiv.children());
        ddiv.find(".container").find(".brctt").append(cdiv.children());
        return ddiv;
    };
    this.fillProdDiv = function (ddiv, prod) {
        ddiv.find(".linkspan").attr("clink", prod.link);
        var domain = FAI_API.getDomainByForward(prod.seller.id);
        var id2show = $.isEmptyObject(domain) ? CUtil.getShortPId(prod.seller.id) : (domain.alias ? domain.alias + " (" + domain.domain + ")" : domain.domain);
        var idtype = $.isEmptyObject(domain) ? "" : "(" + TR("domain") + ")";
        ddiv.find(".id").find(".text").html(id2show);
        ddiv.find(".id").find(".text").attr("fullid", prod.seller.id);
        if (!$.isEmptyObject(domain))
            ddiv.find(".id").find(".text").attr("domain", domain.domain);
        ddiv.find(".id").find(".idtype").html(idtype);
        ddiv.find(".prdname").html(prod.name);
        ddiv.find(".prc").html(fai + prod.price);
        if (prod.expiretime) {
            var vt = new Date(prod.expiretime * 1000);
            ddiv.find(".expt").html(TR("Valid until: ") + CUtil.dateToShortString(vt));
        }
        if (prod.intro)
            ddiv.find(".intro").html(CUtil.escapeHtml(prod.intro));
        if (typeof prod.icon !== "undefined") {
            var ctt = FAI_API.getContentByLink(prod.icon);
            var cttP = CUtil.parseCtt(ctt);
            var idiv;
            if (cttP.fdata) {
                if ($.inArray(cttP.ftype, CONTENT_FILE_TYPE.image) >= 0) {
                    idiv = $("#image-tpl").clone(true, true);
                    idiv.find("img").attr("id", CLink.setString(ctt.clink).toHtmlId());
                    idiv.find("img").attr("type", cttP.ftype);
                    idiv.find("img").attr("src", this.createImgSrc(cttP.ftype, cttP.fdata));
                }
                else if ($.inArray(cttP.ftype, CONTENT_FILE_TYPE.video) >= 0 || $.inArray(cttP.ftype, CONTENT_FILE_TYPE.audio) >= 0) {
                    idiv = $("#video-tpl").clone(true, true);
                } else
                {
                    idiv = $("#image-tpl").clone(true, true);
                    idiv.find(".ctt-rmdr").removeClass("hide");
                }
                ddiv.find(".ctt").html("").append(idiv.children());
            }
        }
        if (prod.tags.length > 0) {
            ddiv.find(".ctt").append($('<div />').addClass("clear").addClass("fullwidth").css("height", 10));
            for (var i in prod.tags) {
                var tdiv = $("<span />").addClass("ctt-rmdr").html(prod.tags[i]);
                ddiv.find(".ctt").append(tdiv);
            }
        }
        return ddiv;
    };
    this.notifyBlock = function (b) {
        $('#cblc').html(b.blockHeight);
        if (currentTab === "br-new-btn")
            $('#getnew-btn').removeClass("hide");
        else
            $('#getnew-btn').addClass("hide");
    };
    this.notifyAccount = function (b) {
        this.updateBalance();
        $("#navi-bar").find(".container").find(".navi-name").html(gParam.accountID).attr("fullid", gParam.accountID);
    };
    this.registerNotifications = function () {
        var aa = function (a) {
            CPage.notifyBlock(a);
        };
        var ab = function (a) {
            CPage.notifyAccount(a);
        };
        FAI_API.regNotifyBlocks(aa);
        FAI_API.regNotifyAccount(ab);
    }
    this.initDatePickerOptions = function () {
        var tomorrow = new Date();
        tomorrow.setDate(tomorrow.getDate() + 1);
        var options = {
            altFormat: '@',
            dateFormat: 'yy-mm-dd',
            minDate: tomorrow
        };
        if (langCode == "zh") {
            for (var k in $.datepicker.regional["zh-CN"])
                options[k] = $.datepicker.regional["zh-CN"][k];
        }
        return options;

    }
};