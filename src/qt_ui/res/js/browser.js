var CONTENT_TYPE_DEFAULT = 0;
var CONTENT_TYPE_FEED = 1;
var CONTENT_TYPE_FOLLOW = 2;
var CONTENT_TYPE_MINE = 3;
var CONTENT_TYPE_HOMEPAGE = 4;
var CONTENT_TYPE_HOT = 5;
var CONTENT_FILE_TYPE = {image: IMAGE_FILE_TYPE, video: VIDEO_FILE_TYPE, audio: AUDIO_FILE_TYPE};
var bufferedFile = {};
var currentFeeRate = {sgstfeer: false, minfeer: 1000.0};
var COIN = 1000000;
var K = 1000;
var tagCount = 0;
var imageParam = {frblk: null, maxc: 15};
(function ($) {
    $.fn.hasScrollBar = function () {
        if (typeof this.get(0) === "undefined")
            return false;
        return this.get(0).scrollHeight > this.height();
    }
})(jQuery);
var CBrowser = new function () {
    var CBrowser = this;
    this.showFullId = function (div, fullId) {
        div.find("a.text").html(CUtil.getShortPId(fullId));
        div.find("li").find("a").css('display', 'inline-block');
    };
    this.hideFullId = function (div, fullId) {
        div.find("a.text").html(CUtil.getShortPId(fullId));
        div.find("li").find("a").hide();
    };
    this.toggleFullId = function (div) {
        var fullId = div.find("a.text").attr("fullid");
        if (CUtil.getShortPId(fullId) === div.find("a.text").html())
            this.hideFullId(div, fullId);
        else
            this.showFullId(div, fullId);
    };
    this.toggleIdOpt = function (div) {
        if (div.hasClass('showbtn')) {
            div.find("li").find("a").hide();
            div.removeClass('showbtn');
        } else {
            div.find("li").find("a").css('display', 'inline-block');
            div.addClass('showbtn');
        }
    };
    this.getFileContentFrJson = function (json) {
        try {
            var ctt = json.content[0];
            var k;
            for (k in ctt.content) {
                if (ctt.content[k].cc_name == "CC_FILE_CONTENT" || ctt.content[k].cc_name == "CC_FILE")
                    return ctt.content[k].content;
            }
        } catch (e) {
            console.log("getFileContentFrJson error");
        }

    };
    this.getB64DataFromLink = function (clink) {
        var ctt = (FAI_API.getContentByLink(clink));
        var cttP = CUtil.parseCtt(ctt);
        var r = cttP.fdata ? cttP.fdata : "";
        return r;
    };
    this.createImgSrc = function (type, b64) {
        return   "data:" + type + ";base64," + b64;
    };
    this.getFileTypeFrJson = function (ctt) {
        var isFile = ctt.content[0].cc_name === "CC_FILE_P";
        if (!isFile)
            return false;
        for (var i in ctt.content[0].content) {
            if (ctt.content[0].content[i].cc_name === "CC_FILE_TYPESTRING")
                return atob(ctt.content[0].content[i].content);
        }
        return null;
    };
    this.getTextFrJson = function (ctt) {
        if (ctt.content[0].cc_name !== "CC_FILE_P" || ctt.content[0].cc_name !== "CC_P")
            return false;
        for (var i in ctt.content[0].content) {
            if (ctt.content[0].content[i].cc_name === "CC_TEXT") {
                var tmp = base64.decode(ctt.content[0].content[i].content);
                return CUtil.escapeHtml(tmp);
            }
        }
        return null;
    };
    this.getLinkFrLinkctt = function (ctt) {
        if (ctt.content[0].cc_name !== "CC_LINK_P") {
            if (ctt.content[0].cc_name !== "CC_LINK")
                return "";
            else
                return ctt.content[0].content[i].link;
        }
        for (var i in ctt.content[0].content) {
            if (ctt.content[0].content[i].cc_name === "CC_LINK")
                return ctt.content[0].content[i].link;
        }
        return "";
    };
    this.decodeRawLink = function (rl) {


//        link.unserializeConst(content)
//        link.ToString();
    };
    this.getLinknameFrLinkctt = function (ctt) {
        if (ctt.content[0].cc_name !== "CC_LINK_P")
            return "";
        for (var i in ctt.content[0].content) {
            if (ctt.content[0].content[i].cc_name === "CC_NAME")
                return base64.decode(ctt.content[0].content[i].content);
        }
        return "";
    };
    this.createImgHtml = function (cttP) {
        return CPage.createImgHtml(cttP.ftype, cttP.fdata);
    };
    this.toggleCmt = function (div) {
        var s = div.find("a.shrt").hasClass("short");
        if (s) {
            var linkstr = div.find("li.linkspan").attr("clink");
            div.find("li.linkspan").html(this.getLinkA(linkstr));
            div.find("li").find("a").show().click(function () {
                CBrowser.regLink($(this));
            });
            div.find("a.shrt").removeClass("short");
        } else {
            div.find("li").find("a").hide();
            div.find("a.shrt").addClass("short");
        }
    };
    this.regLink = function (div) {
        if (typeof div.attr("href") !== "undefined") {
            if (CUtil.isLinkFai(div.attr("href")))
                CBrowser.viewLink(div.attr("href"));
        }
    };
    this.getLinkA = function (l) {
        var ldiv = $("#linka").clone(true, true);
        ldiv.find("a").attr("href", l).html(l).show();
        return ldiv.html();
    };
    this.fillSdiv = function (sdiv, ctt, fType) {
        var id = this.getIdFrContent(ctt);
        sdiv.find(".id").find(".text").attr("fullid", id);
        var domain = this.getDomainFrContent(ctt);
        var id2show = domain ? (domain.alias ? base64.decode(domain.alias) + " (" + domain.domain + ")" : domain.domain) : CUtil.getShortPId(id);
        id2show = CUtil.isIdDev(id) ? TR("Φ Developers") : id2show;
        var idtype = domain ? "(" + TR("domain") + ")" : "";
        sdiv.find(".id").find(".text").html(id2show);
        sdiv.find(".id").find(".idtype").html(idtype);
        sdiv.find(".linkspan").attr("clink", ctt.link);
        var pcc = ctt.content[0].cc_name;
        if (pcc !== "CC_P" && pcc !== "CC_FILE_P" && pcc !== "CC_TEXT_P" && pcc !== "CC_LINK_P" && pcc !== "CC_TEXT" && pcc !== "CC_LINK" && pcc !== "CC_FILE_PACKAGE_P")
            return false;
        if (typeof fType !== "undefined") {
            if (fType === CONTENT_TYPE_HOT) {
                var d = $("<span />").html(TR("Deposit: ") + fai + ctt.satoshi / COIN).css("margin-left", 20);
                sdiv.find(".ctt2").append(d);
            }
        }
        var r = CUtil.parseCtt(ctt);
        if (domain) {
            sdiv.find(".id").find(".text").attr("domain", domain.domain);
            if (domain.icon) {
                var iconCtt = FAI_API.getContentByLink(domain.icon);
                var iconCttP = CUtil.parseCtt(iconCtt);
                var icon = this.createImgHtml(iconCttP);
                sdiv.find(".icon").html(icon);
            }
        }

        var idiv;
        if (r.filepackage) {
            idiv = $("#image-tpl").clone(true, true);
            idiv.find(".ctt-rmdr").removeClass("hide").html(TR("Filepackage")).click(function () {
                FAI_API.goToCustomPage(r.selflink);
            });
        } else if (r.fdata) {
            if ($.inArray(r.ftype, CONTENT_FILE_TYPE.image) >= 0) {
                idiv = $("#image-tpl").clone(true, true);
                idiv.find("img").attr("id", CLink.setString(ctt.clink).toHtmlId());
                idiv.find("img").attr("type", r.ftype);
                var head = base64.decode(r.fdata).substr(0, 3);
                if (head.toLowerCase() === "gif") {
                    idiv.find(".ctt-rmdr").removeClass("hide").attr("fdata", r.fdata).attr("fname", r.fname).html(TR("GIF File")).click(function () {
                        var fdata = base64.decode($(this).attr("fdata"));
                        var fname = $(this).attr("fname");
                        FAI_API.writeFile2(fname, fdata);
                    });
                } else
                    idiv.find("img").attr("src", this.createImgSrc(r.ftype, r.fdata));
            } else if ($.inArray(r.ftype, CONTENT_FILE_TYPE.video) >= 0 || $.inArray(r.ftype, CONTENT_FILE_TYPE.audio) >= 0) {
                idiv = $("#video-tpl").clone(true, true);
            } else
            {
                idiv = $("#image-tpl").clone(true, true);
                idiv.find(".ctt-rmdr").removeClass("hide").attr("fdata", r.fdata).attr("fname", r.fname).html(TR("File")).click(function () {
                    var fdata = base64.decode($(this).attr("fdata"));
                    var fname = $(this).attr("fname");
                    FAI_API.writeFile2(fname, fdata);
                });
            }
        } else
            idiv = $("#image-tpl").clone(true, true);
        if (r.text)
            idiv.find(".text").html(r.text);
        if (r.ltype) {
            if (r.ltype === "BLOCKCHAIN") {
                var lname = r.lname.length > 0 ? r.lname : r.ldata;
                idiv.find(".bclink").html(lname).attr("href", r.ldata).attr("title", r.ldata);
            } else if (r.ltype === "HTTP") {
                var lname = r.lname.length > 0 ? r.lname : r.ltext;
                idiv.find(".bclink").html(lname).attr("href", r.ltext).attr("title", r.ltext);
            }
        }
        var bl = CUtil.getBalanceLevel(ctt.satoshi / COIN);
        if (typeof ctt.fShowValue !== "undefined") {
            if (ctt.fShowValue)
                sdiv.find(".ctt").append(CPage.getBalanceHtml(bl, ctt.satoshi / COIN));
            else
                sdiv.find(".ctt").append(CPage.getBalanceHtml(bl));
        } else
            sdiv.find(".ctt").append(CPage.getBalanceHtml(bl));
        sdiv.find(".ctt").append(idiv.children());
        return sdiv;
    };
    this.addContent = function (ctt, fType, fPos, fShowProd) {
        fPos = typeof fPos !== 'undefined' ? fPos : true;
        fType = typeof fType !== 'undefined' ? fType : CONTENT_TYPE_FEED;
        fShowProd = typeof fShowProd !== 'undefined' ? fShowProd : true;
        if (fShowProd) {
            if (CUtil.isProd(ctt)) {
                var prod = CUtil.parseProd(ctt);
                this.addProduct(prod);
            }
        }
        if (!ctt)
            return false;
        if (ctt.content.length === 0)
            return false;
        if (ctt.content[0].content === "non-standard")
            return false;
        var sdiv = $("#standard-tpl").clone(true, true);
        var pdiv = $("#poster-tpl").clone(true, true).removeAttr("id");
        var cdiv = $("#cmt-tpl").clone(true, true).removeAttr("id");
        sdiv.find(".container").prepend(pdiv.children());
        sdiv.find(".container").find(".brctt").append(cdiv.children());
        switch (fType) {
            case CONTENT_TYPE_FEED:
                sdiv.find(".id-unfollow-btn").parent().remove();
                sdiv.find()
                break;
            case CONTENT_TYPE_FOLLOW:
                sdiv.find(".id-follow-btn").parent().remove();
                break;
            case CONTENT_TYPE_MINE:
                sdiv.find(".id-follow-btn").parent().remove();
                sdiv.find(".id-unfollow-btn").parent().remove();
                sdiv.find(".id-chat-btn").parent().remove();
                break;
            case CONTENT_TYPE_HOMEPAGE:
                sdiv.find(".id-homepage-btn").parent().remove();
                break;
            case CONTENT_TYPE_HOT:
                sdiv.find(".id-unfollow-btn").parent().remove();
                var t = new Date(ctt.time * 1000);
                sdiv.find(".ctt2").append(TR("Posted: ") + CUtil.dateToShortString(t));
                break;
            case CONTENT_TYPE_DEFAULT:
            default:
                break;
        }
        sdiv.removeAttr("id");
        ctt.fShowValue = false;
        sdiv = this.fillSdiv(sdiv, ctt, fType);
        if (sdiv === false)
            return false;
        fPos ? $("#mainframe").prepend(sdiv.children()) : $("#mainframe").append(sdiv.children());
        return true;
    };
    this.addDomain = function (domain, fPos) {
        fPos = typeof fPos !== 'undefined' ? fPos : false;
        var ddiv = $("#domain-tpl").clone(true, true).removeAttr("id").removeClass("hide");
        var pdiv = $("#poster-tpl").clone(true, true).removeAttr("id");
        ddiv.find(".container").prepend(pdiv.children());
        if (domain.forward.target)
            ddiv.find(".id").find(".text").attr("fullid", domain.forward.target);
        else {
            ddiv.find(".id-homepage-btn").parent().remove();
            ddiv.find(".id-copyid-btn").parent().remove();
        }
        var id2show = (domain.alias ? domain.alias + " (" + domain.domain + ")" : domain.domain);
        var idtype = domain ? "("+TR("domain")+")" : "";
        ddiv.find(".id").find(".text").html(id2show).attr("domain", domain.domain);
        ddiv.find(".id").find(".idtype").html(idtype);
        ddiv.find(".alias").html(domain.alias);
        ddiv.find(".domain").html(domain.domain);
        var vt = new Date(domain.expireTime * 1000);
        ddiv.find(".expt").html(CUtil.dateToShortString(vt));
        ddiv.find(".owner").html(domain.owner);
        var bl = CUtil.getBalanceLevel(Number(domain.lockvalue / COIN));
        ddiv.find(".lckvl").html(CPage.getBalanceHtml(bl, Number(domain.lockvalue / COIN)).html());
        ddiv.find(".fwd-tgt").html(domain.forward.target);
        if (domain.icon) {
            var iconCtt = FAI_API.getContentByLink(domain.icon);
            var iconCttP = CUtil.parseCtt(iconCtt);
            var icon = this.createImgHtml(iconCttP);
            ddiv.find(".icon").html(icon);
        }
        fPos ? $("#mainframe").prepend(ddiv.children()) : $("#mainframe").append(ddiv.children());
        return true;
    };
    this.addProduct = function (prod, fPos) {
        fPos = typeof fPos !== 'undefined' ? fPos : false;
        var ddiv = CPage.prepareProdDiv();
        ddiv = CPage.fillProdDiv(ddiv, prod);
        fPos ? $("#mainframe").prepend(ddiv.children()) : $("#mainframe").append(ddiv.children());
        return true;
    };
    this.getDomainFrContent = function (ctt) {
        if (typeof ctt.poster === "undefined")
            return false;
        for (var i in ctt.poster) {
            if (typeof ctt.poster.domain !== "undefined")
                return ctt.poster.domain;
        }
    };
    this.getIdFrContent = function (ctt) {
        if (typeof ctt.poster === "undefined")
            return false;
        for (var i in ctt.poster) {
            if (typeof ctt.poster.id !== "undefined")
                return ctt.poster.id;
        }
    };
    this.replaceContent = function (ctt, fType, sdiv) {
        var id = this.getIdFrContent(ctt);
        sdiv.find(".id").find(".text").attr("fullid", id);
        var domain = this.getDomainFrContent(ctt);
        var id2show = domain ? (domain.alias ? domain.alias + " (" + domain.domain + ")" : domain.domain) : CUtil.getShortPId(id);
        sdiv.find(".id").find(".text").html(id2show);
        sdiv.find(".linkspan").attr("clink", ctt.link);
        return this.fillSdiv(sdiv, ctt);
    };
    this.cttText = function (t) {
        var div = $("#textSpan").find(".text").html(t);
        return div;
    };
    this.clearContent = function () {
        $("#mainframe").children(".container").remove();
        $("#mainframe").children("hr").remove();
    };
    this.refreshNew = function () {
        var ctts = this.getContents(true, "new");
        this.clearContent();
        $(".column2").scrollTop(); // this line fixes scroll bar stay bottom issue
        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FEED, false);
        }
    };
    this.refreshHot = function (fKeyword) {
        fKeyword = typeof fKeyword === "undefined" ? true : fKeyword;
        var params = {};
        var tags = $('#input-search').val();
        tags = (tags.length > 0 && fKeyword) ? tags.split(",") : [];
        params.tags = tags;
        params.cformat = 6;
        var stype = $("#search input[name='srch-type']:checked").val();
        this.clearContent();
        if (stype === "ctt") {
            var ctts = FAI_API.getPromotedContents(params);
            $(".column2")[0].scrollHeight; // this line fixes scroll bar stay bottom issue
            for (var k in ctts)
                this.addContent(ctts[k], CONTENT_TYPE_HOT, false);
            if (typeof ctts.length === "undefined")
                CPage.showNotice(TR("No result, please change the key word"));
        }
        else if (stype === "dmn") {
            var domains = FAI_API.getDomainsByTags(tags);
            for (var k in domains)
                this.addDomain(domains[k]);
            if (domains.length <= 0)
                CPage.showNotice(TR("No result, please change the key word"));
        } else if (stype === "prd") {
            var params = {};
            params.tags = tags;
            var prods = FAI_API.searchProducts(params);
            for (var k in prods)
                this.addProduct(prods[k]);
            if (!prods)
                CPage.showNotice(TR("No result, please change the key word"));
        }
    }
    this.refreshOld = function () {
        var ctts = this.getOldContents("new");
        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FEED, false);
        }
    };
    this.refreshNewFollowed = function () {
        var flist = this.getFollowed();
        if (flist.length == 0) {
            CPage.showNotice(TR('You need to follow someone first'));
            return;
        }
        this.refreshFollowList(flist);
        var ctts = this.getNewContents("fll", flist);
        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FOLLOW, false);
        }
    };
    this.refreshNewMypage = function () {
        var myid = [FAI_API.getAccountID()];
        if (myid.length == 0) {
            CPage.showNotice(TR('You need to register ID first'));
            return;
        }
        var ctts = this.getNewContents("myp", myid);
        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_MINE, false);
        }
    };
    this.refreshNewDevpage = function () {
        var devid = this.getDevId();
        var myid = [devid];
        var ctts = this.getNewContents("myp", myid);
        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_MINE, false);
        }
    };
    this.getDevId = function () {
        var ctt = FAI_API.getContentByLink("fai:0.0.0");
        return ctt.addr;
    };
    this.refreshOldFollowed = function () {
        var flist = this.getFollowed();
        var ctts = this.getOldContents("fll", flist);
        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FOLLOW, false);
        }
    };
    this.refreshOldMypage = function () {
        var myid = [FAI_API.getAccountID()];
        var ctts = this.getOldContents("myp", myid);
        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_MINE, false);
        }
    };
    this.fillHpheader = function (hpheader, id, dm) {
        dm = typeof dm === "undefined" ? {} : dm;
        var icon = "";
        var intro = "";
        if (!$.isEmptyObject(dm)) {
            var iconLinkStr = typeof dm.icon === "undefined" ? "" : (dm.icon.length > 0 ? dm.icon : "");
            if (iconLinkStr.length > 0) {
                var iconCtt = FAI_API.getContentByLink(iconLinkStr);
                var cttP = CUtil.parseCtt(iconCtt);
                icon = this.createImgHtml(cttP);
            }
            var name = typeof dm.alias === "undefined" ? dm.domain : (dm.alias.length > 0 ? dm.alias + " (" + dm.domain + ")" : dm.domain);
            intro = dm.intro.length > 40 ? dm.intro.substr(0, 40) + "..." : dm.intro;
        } else {
            var name = id;
        }
        hpheader.find(".navi-icon").html(icon);
        hpheader.find("#navi-name").find(".navi-name").html(name).attr("fullid", id).attr("domain", dm.domain);
        hpheader.find(".navi-intro").html(intro);
        var balance = FAI_API.getBalance(id).balance;
        var bl = CUtil.getBalanceLevel(balance);
        hpheader.find("#blclvl").html(CPage.getBalanceHtml(bl));
        return hpheader;
    };
    this.goToHomepage = function (id) {
        currentPage = "homepage";
        $("#shdr").remove();
        $("#navi-bar").remove();
        var hpheader = $("#hpheader-tpl").clone(true, true);
        hpheader.attr("id", "hpheader").removeClass("hide");
        var domain = FAI_API.getDomainByForward(id);
        hpheader = this.fillHpheader(hpheader, id, domain);
        $("body").prepend(hpheader);
        $(".navi-name").click(function () {
            CBrowser.toggleIdOpt($(this).parent());
        });
        hpgDisp = [];
        this.refreshHpg(id);
    };
    this.goToLinkpage = function (linkstr, format) {
        currentPage = "link";
        format = typeof format === "undefined" ? 7 : format;
        $("#shdr").remove();
        $("#navi-bar").remove();
        $("#mainframe").html("");
        var ctt = FAI_API.getContentByLink(linkstr);
        this.addContent(ctt);
        for (var i = 0; i <= 7; i++)
            $("#mainframe").append("<a href='" + linkstr + "&format=" + i + "&showdebug=true'>Format " + i + "</a><br />");
        var ctt2 = FAI_API.getContentByLink(linkstr, format);

        var ldiv = $("#linkpage-tpl").clone(true, true).attr("id", "linkpage").removeClass("hide");
        ldiv.find("pre").html(JSON.stringify(ctt2, null, 2));
        if (CUtil.getGet("showdebug")) {
            ldiv.find("input[type='checkbox']").attr("checked", true);
            ldiv.find("#ctt-json").removeClass("hide");
        }
        ldiv.find("input[type='checkbox']").click(function () {

            if ($(this).is(":checked"))
                $(this).parent().find("#ctt-json").removeClass("hide");
            else
                $(this).parent().find("#ctt-json").addClass("hide");
        })
        $("#mainframe").append(ldiv);
    };
    this.getNewContents = function (page, addrs) {
        return this.getContents(true, page, addrs);
    };
    this.getOldContents = function (page, addrs) {

        return this.getContents(false, page, addrs);
    };
    this.getContents = function (fNewOld, page, frAddrs, toAddrs) {
        frAddrs = typeof frAddrs !== 'undefined' ? frAddrs : [];
        toAddrs = typeof toAddrs !== 'undefined' ? toAddrs : [];
        fNewOld = typeof fNewOld !== 'undefined' ? fNewOld : true;
        page = typeof page !== 'undefined' ? page : "new";
        var rc = 10;
        var flink; // from link
        var dParam = {nFile: -1, nPos: -1, nRange: -1, ftxCount: -1, isEnd: false};
        var ctts = [];
        switch (page) {
            case "new":
                flink = CLink.set(fNewOld ? parseInt(FAI_API.getBlockCount()) : newDisp[1]).toString();
                var clink = flink;
                var blkPR = 10; 
                var i = 0;
                while (ctts.length < rc && CLink.set(clink).nHeight > 0) {
                    var tmp = FAI_API.getContents(clink, blkPR, false, frAddrs, toAddrs);
                    if (tmp.length > 0) {
                        var link1 = tmp[0].link;
                        var link2 = tmp[tmp.length - 1].link;
                        clink = CLink.set(link1).nHeight === CLink.set(link2).nHeight ? (CLink.cmp(link1, link2) > 0 ? link1 : link2) : (CLink.cmp(link1, link2) > 0 ? link2 : link1);
                        clink = CLink.set(clink).nVoutPP().toString();
                    } else {
                        clink = CLink.set(CLink.set(clink).nHeight - blkPR).toString();
                        blkPR = blkPR * 10;
                    }

                    ctts = ctts.concat(tmp);
                    if (i++ > 10)
                        break;
                }
                if (clink === "")
                    clink = CLink.set(0).toString();
                newDisp = fNewOld ? [flink, clink] : [newDisp[0], clink];
                break;
            case "myp":
            case "fll":
            case "hpg":
                var param = typeof gParam[page] === "undefined" ? dParam : gParam[page];
                if (param.isEnd)
                    break;
                while (ctts.length < rc) {
                    var tmp = FAI_API.getContentsByAddresses(param.nFile, param.nPos, param.nRange, param.ftxCount, frAddrs, toAddrs, false);
                    if (!tmp.contents)
                        break;
                    ctts = ctts.concat(tmp.contents);
                    param.ftxCount = tmp.totalTxProcessed;
                    param.nFile = tmp.nFile;
                    while (param.ftxCount < tmp.totalTxInRange && ctts.length < rc) {
                        tmp = FAI_API.getContentsByAddresses(param.nFile, param.nPos, param.nRange, param.ftxCount, frAddrs, toAddrs, false);
                        if (!tmp.contents)
                            break;
                        param.ftxCount += tmp.totalTxProcessed;
                        ctts = ctts.concat(tmp.contents);
                    }
                    if (param.ftxCount >= tmp.totalTxInRange) {

                        if (param.nFile > 0)
                            param.ftxCount = -1;
                        else
                            param.isEnd = true;
                        if (param.nFile === 0)
                            break;
                        else
                            param.nFile--;
                    }
                }
                gParam[page] = param;
                break;
        }
        return ctts;
    };
    this.switchTab = function (tabid) {
        if ($("#" + tabid).parent().hasClass("active"))
            return;
        $("#" + tabid).parent().parent().children("li").removeClass("active");
        $("#" + tabid).parent().addClass("active");
        $("#mainframe").find("#search").remove();
        $("#mainframe").children(".container").remove();
        $("#mainframe").children("hr").remove();
        CUtil.initGParam();
        CBrowser.switchTabCore(tabid);
    };
    this.switchTabCore = function (tabid) {
        currentTab = tabid;
        this.clearFollowList();
        switch (tabid) {
            case "br-new-btn":
                this.newAction();
                break;
            case "br-hot-btn":
                this.hotAction();
                break;
            case "br-followed-btn":
                this.followedAction();
                break;
            case "br-mypage-btn":
                this.mypageAction();
                break;
            case "br-dev-btn":
                this.devpageAction();
                break;
            case "pb-simple-btn":
                CPublisher.pubSimpleAction();
                break;
            case "pb-package-btn":
                CPublisher.pubPkgAction();
                break;
            case "pb-largefile-btn":
                CPublisher.pubLFileAction();
                break;
            case "pb-batch-btn":
                CPublisher.pubBatchAction();
                break;
        }
        this.regBottomAction();
    };
    this.newAction = function () {
        newDisp = [];
        if ($("#slider").length == 0)
            CBrowser.refreshImages(true);
        this.refreshNew();
    };
    this.hotAction = function () {
        if ($("#slider").length == 0)
            CBrowser.refreshImages(true);
        var sDiv = $("#search-tpl").clone(true, true);
        $("#mainframe").prepend(sDiv.removeClass("hide").attr("id", "search"));
        $("#search-btn").click(function () {
            CBrowser.refreshHot();
        });
        $("#search input[name='srch-type']").change(function () {
            CBrowser.refreshHot(false);
        });
        this.refreshHot();
    }
    this.followedAction = function () {
        $("#slider").remove();
        this.refreshNewFollowed();
    };
    this.mypageAction = function () {
        $("#slider").remove();
        this.refreshNewMypage();
    };
    this.devpageAction = function () {
        $("#slider").remove();
        this.refreshNewDevpage();
    };
    this.bottomAction = function () {
        var tabid = this.getCurrentTab();
        switch (tabid) {
            case "br-new-btn":
                this.refreshOld();
                break;
            case "br-followed-btn":
                this.refreshOldFollowed("fll");
                break;
            case "br-mypage-btn":
                this.refreshOldMypage();
                break;
        }
        if ($("#hpheader").length > 0) {
            var id = $("#hpheader").find("a.navi-name").attr("fullid");
            this.refreshHpg(id, false);
        }
    };
    this.refreshHpg = function (id, fNewOld) {
        fNewOld = typeof fNewOld !== 'undefined' ? fNewOld : true;
        var ctts = this.getContents(fNewOld, "hpg", [id]);
        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_HOMEPAGE, false);
        }
    };
    this.addSlider = function () {
        if ($("#slider").length === 0) {
            var sldiv = $("#slider-tpl").clone(true, true).attr("id", "slider").removeClass("hide");
            $(".slideholder").html("").append(sldiv);
        }
    };
    this.getCurrentTab = function () {
        return $("#navi-bar").find(".tabbar").find(".active").find("a").attr("id");
    };
    this.refreshImages = function (fFrBuffer, cbh) {
        this.addSlider();
        var app = "browser";
        var path = "buffer";
        var filename = "sliderBuffer.json";
        fFrBuffer = typeof fFrBuffer === "undefined" ? false : fFrBuffer;
        var imgs;
        var writeimg=function(imgs){
            var tmp = {imgs: imgs, frblk: imageParam.frblk};
            var jtmp = JSON.stringify(tmp);
            FAI_API.writeFile(app, path, filename, jtmp);
        }
        var json = FAI_API.readFile(app, path, filename);
        if (!json){
            imgs = CBrowser.getNewImages(imageParam.maxc);
            imageParam.frblk = Number(FAI_API.getBlockCount());
            writeimg(imgs);
        }
        else {
            var tmp = JSON.parse(json);
            imageParam.frblk = tmp.frblk;
            imgs = tmp.imgs;
            if (imgs.length <= 0){
                imgs = CBrowser.getNewImages(imageParam.maxc);
                  imageParam.frblk = Number(FAI_API.getBlockCount());
                  writeimg(imgs);
            }
        }
        if (!fFrBuffer) {
            cbh = typeof cbh === "undefined" ? Number(FAI_API.getBlockCount()) : cbh;
            if (imageParam.frblk) {
                if (imageParam.frblk === cbh)
                    return;
            }
            var maxb = imageParam.frblk ? cbh - imageParam.frblk : 100;
            var newimgs = CBrowser.getNewImages(imageParam.maxc, maxb);
            if (newimgs) {
                if (newimgs.length === imageParam.maxc)
                    imgs = newimgs;
                else {
                    var tmp = imgs;
                    imgs = newimgs;
                    for (var i in tmp) {
                        if (imgs.length >= imageParam.maxc)
                            break;
                        imgs.push(tmp[i]);
                    }
                }
            }
            imageParam.frblk = cbh;
        }
        if (!imgs)
            return;
        if (imgs.length > 0) {
            CBrowser.addSlideImage(imgs);
            this.prepareSlider();
            writeimg(imgs);
        }
        $("a").click(function () {
            CBrowser.regLink($(this));
        });
    };
    this.getNewImages = function (maxc, maxb) {
        maxc = typeof maxc === "undefined" ? 100 : maxc;
        maxb = typeof maxb === "undefined" ? 100 : maxb;
        var ctts = FAI_API.getImages(parseInt(FAI_API.getBlockCount()), maxb, false, maxc);
        var r = [];
        for (var i in ctts) {
            var cttParsed = CUtil.parseCtt(ctts[i]);
            if (typeof cttParsed.ftype === "undefined")
                continue;
            if ($.inArray(cttParsed.ftype, CONTENT_FILE_TYPE.image) < 0)
                continue;
            var head = base64.decode(cttParsed.fdata).substr(0, 3);
            if (head.toLowerCase() !== "gif")
                r.push(cttParsed);
        }
        return r;
    };
    this.createSliderImage = function (cttParsed) {

        if (!cttParsed.fdata)
            return false;
        if ($.inArray(cttParsed.ftype, CONTENT_FILE_TYPE.image) < 0)
            return false;
        var idiv = $("#s-image-tpl").clone(true, true);
        idiv.find("img").attr("id", CLink.setString(cttParsed.selflink).toHtmlId());
        idiv.find("img").attr("type", cttParsed.ftype);
        idiv.find("img").attr("src", this.createImgSrc(cttParsed.ftype, cttParsed.fdata));
        if (cttParsed.ltype) {
            if (cttParsed.ltype === "BLOCKCHAIN") {
                var lname = cttParsed.lname.length > 0 ? cttParsed.lname : cttParsed.ldata;
                idiv.find("a").attr("href", cttParsed.ldata).html(lname);
            } else if (cttParsed.ltype === "HTTP") {
                var lname = cttParsed.lname.length > 0 ? cttParsed.lname : cttParsed.ltext;
                idiv.find("a").attr("href", cttParsed.ltext).html(lname);
            }
        }
        return idiv.html();
    };
    this.addSlideImage = function (imgs) {
        $("#slider ul").html("");
        for (var i in imgs)
            $("#slider ul").append(this.createSliderImage(imgs[i]));
    };
    this.setFollow = function (str2fl, type) {
        type = (typeof type === "undefined") ? "id" : type;
        var myid = FAI_API.getAccountID();
        var fkey = "followed-" + type;
        var followedstr = FAI_API.getConf("browser", myid, "", fkey);
        if (!followedstr) {
            if (FAI_API.setConf("browser", myid, "", fkey, JSON.stringify([str2fl])))
                return [str2fl];
        }
        else {
            var followed = JSON.parse(followedstr);
            for (var i in followed) {
                if (followed[i] === str2fl)
                    return followed;
            }
            followed.push(str2fl);
            if (FAI_API.setConf("browser", myid, "", fkey, JSON.stringify(followed)))
                return followed;
        }
        return false;
    };
    this.setUnfollow = function (str2unfl, type) {
        type = (typeof type === "undefined") ? "id" : type;
        var myid = FAI_API.getAccountID();
        if (type === "domain") {
            var domain = FAI_API.getDomainInfo(str2unfl);
            if (typeof domain.forward !== "undefined") {
                if (domain.forward.linkType === "ID") {
                    var fid = domain.forward.target;
                    this.setUnfollow(fid);
                }
            }
        }
        var fkey = "followed-" + type;
        var followedstr = FAI_API.getConf("browser", myid, "", fkey);
        if (!followedstr)
            return false;
        else {
            var followed = JSON.parse(followedstr);
            for (var i in followed) {
                if (followed[i] === str2unfl)
                    followed.splice(i, 1);
            }
            if (FAI_API.setConf("browser", myid, "", fkey, JSON.stringify(followed)))
                return true;
        }
        return false;
    };
    this.getFollowed = function (type) {
        type = (typeof type === "undefined") ? "all" : type;
        var myid = FAI_API.getAccountID();
        var r = [];
        if (type === "all") {
            var fkey = "followed-id";
            var followedstr = FAI_API.getConf("browser", myid, "", fkey);
            if (followedstr)
                r = JSON.parse(followedstr);
            var d = this.getFollowed("domain");
            if (typeof d !== "undefined")
                r = r.concat(d);
        } else if (type === "id") {
            var fkey = "followed-" + type;
            var followedstr = FAI_API.getConf("browser", myid, "", fkey);
            r = JSON.parse(followedstr);
        } else if (type === "domain") {
            var fkey = "followed-" + type;
            var domains = JSON.parse(FAI_API.getConf("browser", myid, "", fkey));
            for (var i in domains) {
                var domain = FAI_API.getDomainInfo(domains[i]);
                if (typeof domain.forward === "undefined")
                    continue;
                if (typeof domain.forward.linkType === "undefined")
                    continue
                if (domain.forward.linkType === "ID")
                    r.push(domain.forward.target);
            }
        }
        r = CUtil.cleanNullElem(r);
        return r;
    };
    this.showTopLoader = function () {
        $(".loader.top").removeClass("hide");
    };
    this.showBottomLoader = function () {
        $(".loader.bottom").removeClass("hide");
    };
    this.hideTopLoader = function () {
        $(".loader.top").addClass("hide");
    };
    this.hideBottomLoader = function () {
        $(".loader.bottom").addClass("hide");
    };
    this.sliderId = [];
    this.prepareSlider = function () {
        // slider code from http://codepen.io/zuraizm/pen/vGDHl
        for (var id in CBrowser.sliderId) {
            clearInterval(CBrowser.sliderId[id]);
        }
        CBrowser.sliderId = [];
        var sliderInterval = 5000;
        var sliderAuto = setInterval(function () {
            moveRight();
        }, sliderInterval);
        CBrowser.sliderId.push(sliderAuto);
        var slideCount = $('#slider ul li').length;
        var slideWidth = $('#slider ul li').width();
        var slideHeight = $('#slider ul li').height();
        var sliderUlWidth = slideCount * slideWidth;
        $('#slider').css({width: slideWidth, height: slideHeight});
        $('#slider ul').css({width: sliderUlWidth, marginLeft: -slideWidth});
        $('#slider ul li:last-child').prependTo('#slider ul');
        $('#slider').unbind().mouseover(function () {
            clearInterval(sliderAuto);
        }).mouseleave(function () {
            for (var id in CBrowser.sliderId)
                clearInterval(CBrowser.sliderId[id]);
            sliderAuto = setInterval(function () {
                moveRight();
            }, sliderInterval);
            CBrowser.sliderId.push(sliderAuto);
        });
        function moveLeft() {
            $('#slider ul').animate({
                left: +slideWidth
            }, 200, function () {
                $('#slider ul li:last-child').prependTo('#slider ul');
                $('#slider ul').css('left', '');
            });
        }
        function moveRight() {
            $('#slider ul').animate({
                left: -slideWidth
            }, 200, function () {
                $('#slider ul li:first-child').appendTo('#slider ul');
                $('#slider ul').css('left', '');
            });
        }
        $('a.control_prev').unbind().click(function () {
            moveLeft();
        });
        $('a.control_next').unbind().click(function () {
            moveRight();
        });
    };
    this.getPageName = function () {
        if (CUtil.getGet("share"))
            return "share";
        if (CUtil.getGet("cmt"))
            return "cmt";
        var id = CUtil.getGet("id");
        var link = CUtil.getGet("link");
        if (id !== null)
            return "homepage";
        else if (link !== null)
            return "link";
        else
            return "browser";
    };
    this.viewLink = function (link) {
        var url = "fai:browser/?link=" + link;
        FAI_API.goToCustomPage(url);
    };
    this.regBottomAction = function () {
        if (currentTab === "br-hot-btn") {
            $("#getold-btn").addClass("hide");
            return;
        }
        $(".column2").scroll(function () {
            if ($(".column2").scrollTop() + $(".column2").height() >= $(".column2")[0].scrollHeight) {
                CBrowser.bottomAction();
            }
        });
        if (!$(".column2").hasScrollBar()) {
            $("#getold-btn").removeClass("hide").click(function () {
                CBrowser.bottomAction();
                if ($(".column2").hasScrollBar())
                    $(this).addClass("hide");
            });
        } else
            $("#getold-btn").addClass("hide");
    };
    this.refreshFollowList = function (flist) {
        $("#list").html($("<div />").html(TR("Follow List")).addClass("flheader"));
        for (var i in flist) {
            var url = "fai:browser/?id=" + flist[i];
            var f = $("<a />").html(flist[i]).attr("href", url).click(function () {
                FAI_API.goToCustomPage(url);
            });
            $("#list").append($("<div />").html(f));
        }
    };
    this.clearFollowList = function () {
        $("#list").html("");
    }
};
var CPublisher = new function () {
    var CPublisher = this;
    this.getLink = function () {
        return $('#input-link').val();
    };
    this.getTags = function () {
        var r = [];
        $('.input-tag').each(function () {
            if ($(this).val() !== "")
                r.push($(this).val());
        });
        return r;
    };
    this.ctts = [];
    this.handleBatchFiles = function (files, type) {
        CPublisher.ctts = [];
        for (var i in files) {
            var f = files[i];
            var l = i == files.length - 1;
            if (f.name) {
                var r = new FileReader();
                r.readAsDataURL(f);
                r.onload = (function (f, l) {
                    return function (e) {
                        var raw = e.target.result;
                        var nf = CUtil.decodeDataUrl(raw);
                        nf.name = f.name;
                        var cttH = FAI_API.createFileContent(nf);
                        CPublisher.ctts.push(cttH);
                        if (l) {
                            var feer = FAI_API.getFeeRate(0.15);
                            var r = FAI_API.createTxByContents(CPublisher.ctts, feer);
                            CPage.showNotice(TR(r));
                        }
                    };
                })(f, l);
            }
        }

    };
    this.handleFiles = function (files, type) {
        for (var i = 0, f; f = files[i]; i++)
            this.handleFile(f, type);
    }
    this.handleFile = function (f, type) {
        type = typeof type === "undefined" ? "simple" : type;
        var largeFileLimit = 1000000000;
        var perblockLimit = 1000000;
        if (f.size > perblockLimit && type !== "large") {
            CPage.showNotice(TR('The maximum file size is 1MB'));
            return;
        } else if (f.size > largeFileLimit && type === "large") {
            CPage.showNotice(TR('The maximum file size is 1GB'));
            return;
        }
        if (f.name && f.size <= perblockLimit) {
            var r = new FileReader();
            r.readAsDataURL(f);
            r.onload = (function (f) {
                return function (e) {
                    var raw = e.target.result;
                    var nf = CUtil.decodeDataUrl(raw);
                    var hash = FAI_API.getHash(nf.data);
                    if (hash === bufferedFile.hash)
                        return;
                    nf.name = f.name;
                    var cttH = FAI_API.createFileContent(nf);
                    var ctt = FAI_API.getContentByString(cttH.hex);
                    ctt.poster = {id: FAI_API.getAccountID()};
                    ctt.hex = cttH.hex;
                    $("#" + bufferedFile.hash).parent().find('hr').remove();
                    $("#" + bufferedFile.hash).remove();
                    bufferedFile.ctt = ctt;
                    bufferedFile.hash = hash;
                    CPublisher.showPreview(bufferedFile);
                    $('#pubbtnh').show();
                    $("#confirmpub").removeAttr('disabled');
                    CPublisher.showDetails();
                    CPublisher.refreshFee();
                };
            })(f);
        } else if (f.name && perblockLimit < f.size <= largeFileLimit) {
            var maxFilePartSize = 1000000;
            var parts = Math.ceil(f.size / maxFilePartSize);
            $('#pubbtnh').show();
            $("#confirmpub").removeAttr('disabled');
            CPublisher.showDetails();
            CPublisher.refreshFee(f.size);
            var c = confirm(TR('Please confirm to send file with estimated fee of ') + $("#estfee").html());
            if (!c)
                return;
            var a = function (i) {
                if (i === parts)
                    return;
                var size = Math.min(maxFilePartSize, f.size - i * maxFilePartSize);
                var fp = f.slice(i * maxFilePartSize, i * maxFilePartSize + size);
                fp.name = f.name;
                fp.filetype = f.type;
                var r = new FileReader();
                r.readAsDataURL(fp);
                r.onload = (function (fp, i) {
                    return function (e) {
                        var raw = e.target.result;
                        var nf = CUtil.decodeDataUrl(raw);
                        nf.name = fp.name;
                        nf.type = fp.filetype;
                        nf.nPart = i;
                        var cttH = FAI_API.createFilePartContent(nf);
                        var feer = FAI_API.getFeeRate(0.15);
                        var r = FAI_API.createTxByContent(cttH, feer);
                        if (r.success)
                            var rdiv = fp.name + " part " + nf.nPart + " txid:nvout --- " + r.success + ":0 <br />"
                        else
                            var rdiv = fp.name + " part " + nf.nPart + "failed " + r.error + "<br />";
                        $("body").append(rdiv);
                        i++;
                        a(i);
                    };
                })(fp, i);
            };
            a(0);
        }

    };
    this.handleContent = function (ctt) {
        ctt.link = "";
        $("#confirmpub").removeAttr('disabled');
        var feer = $("#input-feerate").val() > 0 ? $("#input-feerate").val() * COIN / K : 0;
        var deposit = $("#promctt-value").val() > 0 ? $("#promctt-value").val() * COIN : 0;
        var locktime = Math.ceil($("#promctt-date").datepicker("getDate") === null ? 0 : $("#promctt-date").datepicker("getDate").getTime() / 1000 + 86400); // add 24 hour
        var locktime = deposit > 0 ? (locktime > 0 ? locktime : 0) : 0;
        var toId = $("#pubto").find("input[type='text']").val();
        if (toId)
            deposit = locktime = 0;
        var r = FAI_API.createTxByContent(ctt, feer, toId, deposit, locktime);
        if (typeof r !== "undefined") {
            if (r.success) {
                if (r.success.length > 0) {
                    this.clear();
                    CPage.showNotice(TR("Successfully published, will be visible after first confirmation"));
                }
            } else {
                CPage.showNotice(TR(r), "e");
                console.log(r);
            }
        }
    };
    this.handleDragOver = function (evt) {
        evt.stopPropagation();
        evt.preventDefault();
        evt.dataTransfer.dropEffect = 'copy'; // Explicitly show this is a copy.
    };
    this.handleFileInput = function (elemId, type) {
        type = typeof type === "undefined" ? "simple" : type;
        var elem = document.getElementById(elemId);
        if (elem && document.createEvent) {
            var evt = document.createEvent("MouseEvents");
            evt.initEvent("click", true, false);
            elem.dispatchEvent(evt);
        }

        input = document.getElementById('theFile');
        if (input.files.length > 1 && type !== "batch") {
            alert(TR('Only one file is allowed per time'));
            return;
        }

        this.handleFiles(input.files, type);
    };
    this.handleFileSelectLarge = function (evt) {
        CPublisher.handleFileSelect(evt, "large");
    };
    this.handleFileSelectBatch = function (evt) {
        CPublisher.handleFileSelect(evt, "batch");
    };
    this.handleFileSelect = function (evt, type) {
        type = typeof type === "undefined" ? "simple" : type;
        evt.stopPropagation();
        evt.preventDefault();

        if (evt.dataTransfer.files.length > 1 && type !== "batch") {
            alert(TR('Only one file is allowed per time'));
            return;
        }
        if (type === "batch") {
            CPublisher.handleBatchFiles(evt.dataTransfer.files, type);
            return;
        }
        CPublisher.handleFiles(evt.dataTransfer.files, type);
        return;
    };
    this.createTextContent = function (t) {
        var ctthex = FAI_API.createTextContent(t);
        var ctt = FAI_API.getContentByString(ctthex.hex);
        ctt.poster = {id: FAI_API.getAccountID()};
        ctt.hex = ctthex.hex;
        return ctt;
    };
    this.createLinkContent = function (l) {
        var ctthex = FAI_API.createLinkContent(l);
        var ctt = FAI_API.getContentByString(ctthex.hex);
        ctt.poster = {id: FAI_API.getAccountID()};
        ctt.hex = ctthex.hex;
        return ctt;
    };
    this.createTagContent = function (tags) {
        var ctthex = "";
        for (var i in tags) {
            var tgcttH = FAI_API.createTagContent(tags[i]);
            ctthex += tgcttH.hex;

        }
        var ctt = FAI_API.getContentByString(ctthex);
        ctt.hex = ctthex;
        return ctt;
    };
    this.createLangContent = function (lang) {
        var langctt = null;
        if (!lang)
            return null;
        if (lang === "en" || lang === "en_US")
            langctt = FAI_API.icall("encodecontentunit", ["CC_LANG_EN", "", 1])
        else if (lang === "zh" || lang === "zh_CN")
            langctt = FAI_API.icall("encodecontentunit", ["CC_LANG_ZH", "", 1])
        else
            langctt = FAI_API.icall("encodecontentunit", ["CC_LANG", lang, 1])
        return langctt;
    };
    this.combineCtt = function (tctt, lctt, fctt, tgctt, lgctt) { // text content, link, file, tag, lang
        if (tctt && !lctt && !fctt && !tgctt && !lgctt)
            return tctt;
        else if (!tctt && lctt && !fctt && !tgctt && !lgctt)
            return lctt;
        else if (!tctt && !lctt && fctt && !tgctt && !lgctt)
            return fctt;
        else if (!tctt && !lctt && !fctt && tgctt && !lgctt)
            return tgctt;
        else {
            var hex = "";
            if (fctt)
                hex += fctt.hex;
            if (tctt)
                hex += tctt.hex;
            if (lctt)
                hex += lctt.hex;
            if (tgctt)
                hex += tgctt.hex;
            if (lgctt)
                hex += lgctt.hex;
            var ctthex = FAI_API.createPContent(hex);
            var ctt = FAI_API.getContentByString(ctthex.hex);
            ctt.poster = {id: FAI_API.getAccountID()};
            ctt.hex = ctthex.hex;
            return ctt;
        }
    };
    this.addTagField = function () {
        var maxTag = this.getMaxPromTag();
        var tagdiv = $("#pubtag-tpl").clone(true, true).addClass("tag-container");
        var tagId = "tag-" + tagCount;
        var tagChkId = "chktag-" + tagCount;
        tagdiv.removeAttr("id").removeClass("hide").find("input[name='tag']").attr('id', tagId);
        var c = this.getPromTagCount();
        if (c >= maxTag)
            tagdiv.find("input[type='checkbox']").attr("disabled", true);
        tagdiv.find("input[type='checkbox']").attr('id', tagChkId).change(function () {
            CPublisher.refreshPromTag();
        });
        if ($("#promctt").length === 0)
            $(tagdiv).insertBefore($('#pubbtnh'));
        else
            $(tagdiv).insertBefore($('#promctt'));
        tagCount++;
    };
    this.getPromTagCount = function () {
        var r = 0;
        $(".tag-container").find("input[name='tag']").each(function () {
            var tagChkId = "#chk" + $(this).attr("id");
            if ($(tagChkId).prop("checked"))
                r++;
        });
        return r;
    };
    this.getMaxPromTag = function () {
        var maxTag = $("#promctt-value").val() > 0 ? Math.floor($("#promctt-value").val()) : 10;
        maxTag = Math.min(maxTag, 10);
        return maxTag;
    }
    this.refreshPromTag = function () {
        var maxTag = this.getMaxPromTag();
        var r = this.getPromTagCount();
        if (r >= maxTag) {
            var chkCount = 0;
            $(".tag-container").find("input[name='tag']").each(function () {
                var tagChkId = "#chk" + $(this).attr("id");
                if (!$(tagChkId).prop("checked"))
                    $(tagChkId).attr("disabled", true);
                else {
                    if (++chkCount > maxTag)
                        $(tagChkId).prop("checked", false).attr("disabled", true);
                }
            });
        } else if (r < maxTag) {
            $(".tag-container").find("input[name='tag']").each(function () {
                var t = maxTag - r;
                var tagChkId = "#chk" + $(this).attr("id");
                if ($(tagChkId).attr("disabled") && t > 0) {
                    $(tagChkId).removeAttr("disabled");
                    t--;
                }
            });
        }
    };
    this.addPromCttField = function (fTip) {
        fTip = typeof fTip === "undefined" ? false : fTip;
        var promcttdiv = $("#promctt-tpl").clone(true, true);
        promcttdiv.attr("id", "promctt").removeClass("hide");
        $(promcttdiv).insertBefore($('#pubbtnh'));
        if (fTip) {
            $("promctt-check").remove();
            $(".input-tag-wrapper").removeClass("hide");
            $("#pub-value-span").html(TR("Tip value"));
            $("#promctt-value").attr("placeholder", TR("Tip value"))
            $("#pub-until-span").remove();
            $("#promctt-date").remove();
        } else {

            $("#promctt-value").attr("placeholder", TR("Value locked for ranking"))
            var tomorrow = new Date();
            tomorrow.setDate(tomorrow.getDate() + 1);
            $("#promctt-date").datepicker(CPage.initDatePickerOptions());
        }

    };
    this.addLinkField = function () {
        var linkdiv = $("#publink-tpl").clone(true, true);
        linkdiv.removeAttr("id").removeClass("hide");
        $(linkdiv).insertAfter($('#theText'));
    };
    this.addValidtilField = function () {
        var linkdiv = $("#pubvalid-tpl").clone(true, true);
        linkdiv.removeAttr("id").removeClass("hide");
        $(linkdiv).insertAfter($('#theText'));
    };
    this.addPubToField = function () {
        if ($("#pubto").length <= 0) {
            var pubto = $("#pubto-tpl").clone(true, true);
            pubto.attr("id", "pubto").removeClass("hide");
            $(pubto).insertBefore($('#pubbtnh'));
        }
    };
    this.showPreview = function (file) {
        if (file.ctt.content[0].content === "non-standard")
            return;
        var sdiv = $("#standard-tpl").clone(true, true);
        sdiv.removeAttr("id");
        sdiv.find(".container").attr("id", file.hash);
        var id = CBrowser.getIdFrContent(file.ctt);
        sdiv.find(".id").find(".text").attr("fullid", id);
        sdiv.find(".id").find(".text").html(CUtil.getShortPId(id));
        sdiv.find(".linkspan").attr("clink", file.ctt.link);
        sdiv = CBrowser.fillSdiv(sdiv, file.ctt);
        if (sdiv === false)
            return false;
        $("#pubpreview").removeClass("hide").append(sdiv.children());
        return true;
    };
    this.getInputTags = function () {
        var r = [];
        if ($("#CC_TAG_EROTIC").prop("checked"))
            r.push("CC_TAG_EROTIC");
        if ($("#CC_TAG_HORROR").prop("checked"))
            r.push("CC_TAG_HORROR");
        if ($("#CC_TAG_POLITICS").prop("checked"))
            r.push("CC_TAG_POLITICS");
        $(".pubtf input[name='tag']").each(function () {
            var tagChkId = "#chk" + $(this).attr("id");
            if ($(tagChkId).prop("checked") && $(this).val() !== "")
                r.push($(this).val());
        });
        $(".pubtf input[name='tag']").each(function () {
            var tagChkId = "#chk" + $(this).attr("id");
            if (!$(tagChkId).prop("checked") && $(this).val() !== "")
                r.push($(this).val());
        });
        return r;
    };
    this.getInputLang = function () {
        if (!$("#CC_LANG").prop("checked"))
            return false;
        return $("#CC_LANG").attr("lang");
    };
    this.showDetails = function (fTip) {
        fTip = typeof fTip === "undefined" ? false : fTip;
        $("#cclang-span").html(TR("tag-" + langCode));
        $("#CC_LANG").attr("lang", langCode);
        $("#pubdfttag").removeClass("hide");
        var feediv = $("#fee-tpl")
        feediv.removeAttr("id").removeClass("hide").find("input[type='text']").addClass("visible");
        $(feediv).insertBefore($('#pubbtnh'));
        if ($("#promctt").length === 0) {
            this.addPromCttField(fTip);
        }
    };
    this.refreshFee = function (size) {
        var len = typeof size === "undefined" ? 260 : size;
        len += $("#theText").val().length;
        if (typeof bufferedFile.ctt !== "undefined")
            len += bufferedFile.ctt.hex.length / 2 + 100;
        var sgstPK = currentFeeRate.sgstfeer / COIN * K;
        if ($("#input-feerate").val() == "")
            $("#input-feerate").val(sgstPK);
        var feeRatePK = typeof Number($("#input-feerate").val()) !== "number" ? sgstPK : $("#input-feerate").val();
        var minPK = currentFeeRate.minfeer / COIN * K;
        var estfee = feeRatePK * len / K;
        $("#estfee").html("φ" + estfee.toString().substring(0, 8));
        $("#sgstr").html(sgstPK.toString().substring(0, 6) + " φ/KB");
        $("#minr").html(minPK.toString().substring(0, 6) + " φ/KB");
    };
    this.addDefaultProUntil = function () {
        if ($("#promctt-date").datepicker("getDate") === null) {
            var tomorrow = new Date();
            tomorrow.setDate(tomorrow.getDate() + 1);
            $("#promctt-date").val($.datepicker.formatDate('yy-mm-dd', tomorrow));
        }
    };
    this.shareLink = function (link) {
        if (currentPage === "homepage"||currentPage === "link") {
            FAI_API.goToCustomPage("fai:browser/?share=true&link=" + link);
            return;
        }
        $('#pubbtnh').show();
        if (!$("#addlink").attr("disabled")) {
            this.addLinkField();
            $("#addlink").attr('disabled', true);
        }
        $("#input-link").val(link);
    };
    this.shareId = function (id) {
        if (typeof currentPage !== "undefined") {
            if (currentPage === "homepage") {
                FAI_API.goToCustomPage("fai:browser/?share=true&id=" + id);
                return;
            }
        }
        $("#theText").val(id + "\n");
        $("#confirmpub").removeAttr('disabled');
        CPublisher.showDetails();
        CPublisher.refreshFee();
    }
    this.togglePromCtt = function () {
        if ($("#promctt").length > 0) {
            $("#promctt").remove();
            this.addPubToField();
        } else if ($("#pubto").length > 0) {
            $("#pubto").remove();
            this.addPromCttField();
        } else
            this.addPromCttField();
    };
    this.commentLink = function (link, id, fTip) {
        fTip = typeof fTip === "undefined" ? false : fTip;
        if (currentPage === "homepage"||currentPage === "link") {
            FAI_API.goToCustomPage("fai:browser/?cmt=true&link=" + link + "&id=" + id);
            return;
        }
        this.shareLink(link);
        this.showDetails(fTip);
        this.addPubToField();
        $("#pubto").find("input[type='text']").val(id);
    };
    this.clearWithAlert = function () {
        if ($("#theText").val() !== "") {
            if (confirm(TR('Please confirm to clear unpublished content')))
                this.clear();
        } else
            this.clear();
    };
    this.clear = function () {
        $("#pubpreview").addClass("hide");
        $("#theText").val("");
        $("#promctt-value").val("");
        $("#promctt-date").val("");
        $(".tag-container").remove();
        $(".pubtf input[type='checkbox']").prop("checked", false);
        $("#input-link-name").val("");
        $("#input-link").val("");
        $("#input-feerate").val("");
        if ($("#pubto").length > 0)
            this.togglePromCtt();
    };
    this.regFeeRate = function () {
        currentFeeRate.sgstfeer = FAI_API.getFeeRate(0.15);
        currentFeeRate.minfeer = FAI_API.getFeeRate();
    };
    this.pubSimpleAction = function () {
        $("#mainframe").html($("#maininput-tpl").html()).attr("id", "maininput");
        $("#maininput").html($("#maininput-tpl").html());
        prepareSimplePub();
        prepareStdTpl();
    };
    this.pubPkgAction = function () {
        $("#mainframe").html($("#maininput-tpl").html()).attr("id", "maininput");
        var pkgdiv = $("#pkginput-tpl").clone(true, true);
        $("#maininput").html(pkgdiv.removeClass("hide").attr("id", "pkginput"));
        preparePkgPub();
    };
    this.pubLFileAction = function () {
        $("#mainframe").html($("#maininput-tpl").html()).attr("id", "maininput");
        $("#maininput").html($("#maininput-tpl").html());
        prepareSimplePub("large");
        prepareStdTpl();
    };
    this.pubBatchAction = function () {
        $("#mainframe").html($("#maininput-tpl").html()).attr("id", "maininput");
        $("#maininput").html($("#maininput-tpl").html());
        prepareSimplePub("batch");
        prepareStdTpl();
    };
};