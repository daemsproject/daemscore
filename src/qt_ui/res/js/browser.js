var CONTENT_TYPE_DEFAULT = 0;
var CONTENT_TYPE_FEED = 1;
var CONTENT_TYPE_FOLLOW = 2;
var CONTENT_TYPE_MINE = 3;
var CONTENT_TYPE_HOMEPAGE = 4;
var CONTENT_FILE_TYPE = {image: IMAGE_FILE_TYPE, video: VIDEO_FILE_TYPE, audio: AUDIO_FILE_TYPE};
var bufferedFile = {};
var currentFeeRate = {sgstfeer: false, minfeer: 1000.0};
var COIN = 1000000;
var K = 1000;
var tagCount = 0;
(function ($) {
    $.fn.hasScrollBar = function () {
        return this.get(0).scrollHeight > this.height();
    }
})(jQuery);
var CBrowser = new function () {
    var CBrowser = this;
    this.getShortPId = function (fullId) {
        return typeof fullId === "undefined" ? "" : fullId.substr(0, 10) + "..." + fullId.substr(fullId.length - 2);
    };
    this.getLongPId = function (fullId) {
        return fullId;
//        return fullId.substr(0, 25) + "..." + fullId.substr(fullId.length - 2);
    };
    this.showFullId = function (div, fullId) {
        div.find("a.text").html(this.getShortPId(fullId));
        div.find("li").find("a").css('display', 'inline-block');
    };
    this.hideFullId = function (div, fullId) {
        div.find("a.text").html(this.getShortPId(fullId));
        div.find("li").find("a").hide();
    };
    this.toggleFullId = function (div) {
        var fullId = div.find("a.text").attr("fullid");
        if (this.getShortPId(fullId) === div.find("a.text").html())
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
        var cj = (BrowserAPI.getContentByLink(clink));
        var r = this.getFileContentFrJson(cj);
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
        console.log(ctt);

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
        console.log(cttP);
        if (typeof cttP.ftype === "undefined")
            return false;
        if (!cttP.ftype)
            return false;
        if ($.inArray(cttP.ftype, CONTENT_FILE_TYPE.image) < 0)
            return false;
        var idiv = $('<img/>', {
            type: cttP.ftype,
            src: this.createImgSrc(cttP.ftype, cttP.fdata),
        });
        return idiv;
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
//    this.showFullImg = function (div) {
//        if (div.find("img").length !== 0) {
//            $("#fullImage").html(div.html());
//            $("#fullImage").find("img").removeClass("brimg");
//        }
//    };
    this.getLinkA = function (l) {
        var ldiv = $("#linka").clone(true, true);
        ldiv.find("a").attr("href", l).html(l).show();
//        console.log(ldiv.html());
        return ldiv.html();
    };
    this.addContent = function (ctt, fType, fPos) {
//        console.log(ctt);
        fPos = typeof fPos !== 'undefined' ? fPos : true;
        fType = typeof fType !== 'undefined' ? fType : CONTENT_TYPE_FEED;
        if (!ctt)
            return false;

//        console.log(ctt.content);
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
            case CONTENT_TYPE_DEFAULT:
            default:
                break;
        }
        sdiv.removeAttr("id");
        sdiv = this.fillSdiv(sdiv, ctt);
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
        console.log(domain);
        if (domain.forward.target)
            ddiv.find(".id").find(".text").attr("fullid", domain.forward.target);
        else {
            ddiv.find(".id-homepage-btn").parent().remove();
            ddiv.find(".id-copyid-btn").parent().remove();
        }
        var id2show = domain.domain;
        var idtype = domain ? "(domain)" : "";

        ddiv.find(".id").find(".text").html(id2show).attr("domain", domain.domain);
        ddiv.find(".id").find(".idtype").html(idtype);
        ddiv.find(".alias").html(domain.alias);
        ddiv.find(".domain").html(domain.domain);
        var vt = new Date(domain.expireTime * 1000);
        ddiv.find(".expt").html(dateToShortString(vt));
        ddiv.find(".owner").html(domain.owner);
        var bl = this.getBalanceLevel(Number(domain.lockvalue / COIN));
        console.log(bl);
        ddiv.find(".lckvl").html(this.getBalanceHtml(bl, Number(domain.lockvalue / COIN)).html());
        ddiv.find(".fwd-tgt").html(domain.forward.target);
        if (domain.icon) {
            var iconCtt = BrowserAPI.getContentByLink(domain.icon);
            var iconCttP = this.parseCtt(iconCtt);
            var icon = this.createImgHtml(iconCttP);
            console.log(icon);
            ddiv.find(".icon").html(icon);
        }
        fPos ? $("#mainframe").prepend(ddiv.children()) : $("#mainframe").append(ddiv.children());
        return true;
    };
    this.addProduct = function (prod, fPos) {
        fPos = typeof fPos !== 'undefined' ? fPos : false;
        var ddiv = $("#prod-tpl").clone(true, true).removeAttr("id").removeClass("hide");
        var pdiv = $("#poster-tpl").clone(true, true).removeAttr("id");
        var cdiv = $("#cmt-tpl").clone(true, true).removeAttr("id");
        var bdiv = $("#buy-btn-tpl").clone(true, true).removeAttr("id");
        cdiv.find(".cmt").append(bdiv.children());
        ddiv.find(".container").prepend(pdiv.children());
        ddiv.find(".container").find(".brctt").append(cdiv.children());
        ddiv.find(".linkspan").attr("clink", prod.link);
        console.log(prod);
        var domain = BrowserAPI.getDomainByForward(prod.seller.id);
        var id2show = $.isEmptyObject(domain) ? this.getShortPId(prod.seller.id) : domain.domain;
        var idtype = $.isEmptyObject(domain) ? "" : "(domain)";
        console.log(id2show);
        ddiv.find(".id").find(".text").html(id2show);
        ddiv.find(".id").find(".text").attr("fullid", prod.seller.id);
        if (!$.isEmptyObject(domain))
            ddiv.find(".id").find(".text").attr("domain", domain.domain);
        ddiv.find(".id").find(".idtype").html(idtype);
//        var bl = this.getBalanceLevel(ctt.satoshi / COIN);
//        sdiv.find(".ctt").append(this.getBalanceHtml(bl, ctt.satoshi / COIN));
        ddiv.find(".prdname").html(prod.name);
        ddiv.find(".prc").html(fai + prod.price);
        var vt = new Date(prod.expiretime * 1000);
        ddiv.find(".expt").html(dateToShortString(vt));
        ddiv.find(".intro").html(CUtil.escapeHtml(prod.intro));
        if (typeof prod.icon !== "undefined") {
            console.log(prod.icon);
            var ctt = BrowserAPI.getContentByLink(prod.icon);
            var cttP = this.parseCtt(ctt);
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
        var id2show = domain ? domain.domain : this.getShortPId(id);
        sdiv.find(".id").find(".text").html(id2show);
        sdiv.find(".linkspan").attr("clink", ctt.link);
        return this.fillSdiv(sdiv, ctt);

    };
    this.fillSdiv = function (sdiv, ctt) {
        var id = this.getIdFrContent(ctt);
        sdiv.find(".id").find(".text").attr("fullid", id);
        var domain = this.getDomainFrContent(ctt);
        var id2show = domain ? domain.domain : this.getShortPId(id);
        var idtype = domain ? "(" + TR("domain") + ")" : "";

        sdiv.find(".id").find(".text").html(id2show);
        sdiv.find(".id").find(".idtype").html(idtype);
        sdiv.find(".linkspan").attr("clink", ctt.link);
        var pcc = ctt.content[0].cc_name;
        if (pcc !== "CC_P" && pcc !== "CC_FILE_P" && pcc !== "CC_TEXT_P" && pcc !== "CC_LINK_P" && pcc !== "CC_TEXT" && pcc !== "CC_LINK")
            return false;
        var r = this.parseCtt(ctt);
        if (domain) {
            sdiv.find(".id").find(".text").attr("domain", domain.domain);
            if (domain.icon) {
                var iconCtt = BrowserAPI.getContentByLink(domain.icon);
                var iconCttP = this.parseCtt(iconCtt);
                var icon = this.createImgHtml(iconCttP);
                sdiv.find(".icon").html(icon);
            }
        }
        var idiv;
        if (r.fdata) {
            if ($.inArray(r.ftype, CONTENT_FILE_TYPE.image) >= 0) {
                idiv = $("#image-tpl").clone(true, true);
                idiv.find("img").attr("id", CLink.setString(ctt.clink).toHtmlId());
                idiv.find("img").attr("type", r.ftype);
                idiv.find("img").attr("src", this.createImgSrc(r.ftype, r.fdata));
            }
            else if ($.inArray(r.ftype, CONTENT_FILE_TYPE.video) >= 0 || $.inArray(r.ftype, CONTENT_FILE_TYPE.audio) >= 0) {
                idiv = $("#video-tpl").clone(true, true);
            } else
            {
                idiv = $("#image-tpl").clone(true, true);
                console.log("rmdr");
                idiv.find(".ctt-rmdr").removeClass("hide").attr("fdata", r.fdata).attr("fname", r.fname).click(function () {
                    var fdata = base64.decode($(this).attr("fdata"));
                    var fname = $(this).attr("fname");
                    BrowserAPI.writeFile2(fname, fdata);
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
        var bl = this.getBalanceLevel(ctt.satoshi / COIN);
        sdiv.find(".ctt").append(this.getBalanceHtml(bl, ctt.satoshi / COIN));
        sdiv.find(".ctt").append(idiv.children());
        return sdiv;
    };
    this.cttText = function (t) {
        var div = $("#textSpan").find(".text").html(t);
        return div;
    };
    this.refreshNew = function () {
        var ctts = this.getContents(true, "new");
//        console.log(ctts);

        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FEED, false);
        }
    };
    this.refreshHot = function (fKeyword) {
        fKeyword = typeof fKeyword === "undefined" ? true : fKeyword;
        var params = {};
        var tags = $('#input-search').val();
        if (tags.length > 0 && fKeyword)
            tags = tags.split(",");
        else
            tags = [];
        params.tags = tags;
        params.cformat = 6;
        //params.ccs=["CC_TEXT"];
        var stype = $("#search input[name='srch-type']:checked").val();
        console.log(stype);
        $("#mainframe").children(".container").remove();
        $("#mainframe").children("hr").remove();

        if (stype === "ctt") {
            var ctts = BrowserAPI.getPromotedContents(params);
            console.log(ctts);
            for (var k in ctts)
                this.addContent(ctts[k], CONTENT_TYPE_FEED, false);
        }
        else if (stype === "dmn") {
            var domains = BrowserAPI.getDomainsByTags(tags);
            for (var k in domains)
                this.addDomain(domains[k]);
        } else if (stype === "prd") {
            var params = {};
            params.tags = tags;
            var prods = BrowserAPI.searchProducts(params);
            console.log(prods);
            for (var k in prods)
                this.addProduct(prods[k]);
        }
    }
    this.refreshOld = function () {
        var ctts = this.getOldContents("new");
        console.log('refreshOld');
        console.log(ctts);
        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FEED, false);
        }
    };
    this.refreshNewFollowed = function () {
        var flist = this.getFollowed();
        if (flist.length == 0) {
            this.showNotice(TR('You need to follow someone first'));
            return;
        }
        this.refreshFollowList(flist);
        var ctts = this.getNewContents("fll", flist);
        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FOLLOW, false);
        }
    };
    this.refreshNewMypage = function () {
        var myid = [BrowserAPI.getAccountID()];
        if (myid.length == 0) {
            this.showNotice(TR('You need to register ID first'));
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
//       myid = [""];
        console.log(myid);
        var ctts = this.getNewContents("myp", myid);
        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_MINE, false);
        }
    };
    this.getDevId = function () {
        var ctt = BrowserAPI.getContentByLink("fai:0.0.0");
        return ctt.addr;
    };
    this.refreshOldFollowed = function () {
        var flist = this.getFollowed();
//        console.log(flist);
        var ctts = this.getOldContents("fll", flist);
        for (var k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FOLLOW, false);
        }
    };
    this.refreshOldMypage = function () {
        var myid = [BrowserAPI.getAccountID()];
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
                var iconCtt = BrowserAPI.getContentByLink(iconLinkStr);
                var cttP = this.parseCtt(iconCtt);
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
        var balance = BrowserAPI.getBalance(id).balance;
        var bl = CBrowser.getBalanceLevel(balance);
        hpheader.find("#blclvl").html(CBrowser.getBalanceHtml(bl));
        return hpheader;
    }
    this.goToHomepage = function (id) {
        $("#shdr").remove();
        $("#navi-bar").remove();
        var hpheader = $("#hpheader-tpl").clone(true, true);
        hpheader.attr("id", "hpheader").removeClass("hide");
        var domain = BrowserAPI.getDomainByForward(id);
        console.log(domain);
        hpheader = this.fillHpheader(hpheader, id, domain);
        $("body").prepend(hpheader);
        $(".navi-name").click(function () {
            CBrowser.toggleIdOpt($(this).parent());
            console.log('test');
        });
//        console.log(id);      
        hpgDisp = [];
        this.refreshHpg(id);
    };
    this.goToLinkpage = function (linkstr, format) {
        format = typeof format === "undefined" ? 7 : format;
        console.log("goto");
        $("#shdr").remove();
        $("#navi-bar").remove();
        $("#mainframe").html("");
        var ctt = BrowserAPI.getContentByLink(linkstr);
        if (!this.addContent(ctt))
            for (var i = 0; i <= 7; i++)
                $("#mainframe").append("<a href='" + linkstr + "&format=" + i + "'>Format " + i + "</a><br />");
        var ctt2 = BrowserAPI.getContentByLink(linkstr, format);
        console.log(linkstr);
//        var url = "fai:browser/?link=" + linkstr;

        var ldiv = $("#linkpage-tpl").clone(true, true).attr("id", "linkpage").removeClass("hide");
        ldiv.find("pre").html(JSON.stringify(ctt2, null, 2));
        ldiv.find("input[type='checkbox']").click(function () {

            if ($(this).is(":checked"))
                $(this).parent().find("#ctt-json").removeClass("hide");
            else
                $(this).parent().find("#ctt-json").addClass("hide");
        })
        $("#mainframe").append(ldiv);

    };
//    this.regFileDownload = function () {
//        console.log("565");
//        console.log($(".ctt-rmdr").length);
//        $(".ctt-rmdr").unbind().click(function () {
//            console.log("cli");
//        });
//    }
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
        var rc = 2;
        var flink; // from link
        var dParam = {nFile: -1, nPos: -1, nRange: -1, ftxCount: -1, isEnd: false};
        var ctts = [];
        switch (page) {
            case "new":
                flink = CLink.set(fNewOld ? parseInt(BrowserAPI.getBlockCount()) : newDisp[1]).toString();
//        console.log(fNewOld + " " + sbh);
                var clink = flink;
                console.log(flink);
                var blkPR = 10;  // todo change to about 10
                var i = 0;
                while (ctts.length < rc && CLink.set(clink).nHeight > 0) {
                    var tmp = BrowserAPI.getContents(clink, blkPR, false, frAddrs, toAddrs);
                    if (tmp.length > 0) {
                        var link1 = tmp[0].link;
                        var link2 = tmp[tmp.length - 1].link;
                        clink = CLink.set(link1).nHeight === CLink.set(link2).nHeight ? (CLink.cmp(link1, link2) > 0 ? link1 : link2) : (CLink.cmp(link1, link2) > 0 ? link2 : link1);
                        clink = CLink.set(clink).nVoutPP().toString();
//                console.log(link1 + "   " + link2);
                    } else {
                        clink = CLink.set(CLink.set(clink).nHeight - blkPR).toString();
                        blkPR = blkPR * 10;
                    }

                    console.log(clink + "   " + tmp.length + "  " + ctts.length);
                    ctts = ctts.concat(tmp);
                    if (i++ > 10)
                        break;
                }
                if (clink === "")
                    clink = CLink.set(0).toString();
                newDisp = fNewOld ? [flink, clink] : [newDisp[0], clink];
                console.log(newDisp + "  " + new Date());
                break;
//            case "fll":
//                fllDisp = fNewOld ? [flink, clink] : [fllDisp[0], clink];
//                console.log(fllDisp);
//                break;


            case "myp":
            case "fll":
            case "hpg":

//                var param = mypDisp.length > 0 ? (fNewOld ? dParam : mypDisp[0]) : dParam;
                var param = typeof gParam[page] === "undefined" ? dParam : gParam[page];
                if (param.isEnd)
                    break;
                while (ctts.length < rc) {
                    var tmp = BrowserAPI.getContentsByAddresses(param.nFile, param.nPos, param.nRange, param.ftxCount, frAddrs, toAddrs, false);
                    console.log("call count out " + tmp.contents.length);
                    console.log(tmp);
                    if (!tmp.contents)
                        break;
                    ctts = ctts.concat(tmp.contents);
                    param.ftxCount = tmp.totalTxProcessed
                    param.nFile = tmp.nFile;
                    while (param.ftxCount < tmp.totalTxInRange && ctts.length < rc) {
                        tmp = BrowserAPI.getContentsByAddresses(param.nFile, param.nPos, param.nRange, param.ftxCount, frAddrs, toAddrs, false);
                        console.log("call count in " + tmp.contents.length);
                        console.log(tmp);
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
                console.log(param);
                gParam[page] = param;
                console.log(JSON.stringify(gParam));
                break;
//            case "hpg":
//                hpgDisp = fNewOld ? [flink, clink] : [hpgDisp[0], clink];
//                console.log(hpgDisp);
//                break;
        }





//        console.log(ctts);
        return ctts;

    };

    this.switchTab = function (tabid) {
        if ($("#" + tabid).parent().hasClass("active"))
            return;
        console.log(tabid);
        $("#" + tabid).parent().parent().children("li").removeClass("active");
        $("#" + tabid).parent().addClass("active");
        $("#mainframe").find("#search").remove();
        $("#mainframe").children(".container").remove();
        $("#mainframe").children("hr").remove();
//        this.showTopLoader();
//        setTimeout(function () {
        gParam = {};
        CBrowser.switchTabCore(tabid);
//            this.hideTopLoader();
//        }, tabid);

//        newDisp = [];
    };
    this.switchTabCore = function (tabid) {
        currentTab = tabid;
        console.log(tabid);
        this.clearFollowList();
        switch (tabid) {
            case "br-home-btn":
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
        console.log($("#slider").length);
        if ($("#slider").length == 0)
            CBrowser.refreshImages(true);
        this.refreshNew();
//        this.refreshOld();
    };
    this.hotAction = function () {
        if ($("#slider").length == 0)
            CBrowser.refreshImages(true);
        var sDiv = $("#search-tpl").clone(true, true);
        $("#mainframe").prepend(sDiv.removeClass("hide").attr("id", "search"));
//         $("#search input[name='srch-type',value='ctt']").attr("checked",true);
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
    this.showNotice = function (n, s) {
        s = typeof s !== 'undefined' ? s : 5;
        $("#notices").html(n).show();
        $("#notices").delay(s * 1000).hide(0);
    };
    this.bottomAction = function () {
        var tabid = this.getCurrentTab();
        switch (tabid) {
            case "br-home-btn":
                this.refreshOld();
                break;
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
        console.log(ctts.length);
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

        console.log("refreshImages");
        this.addSlider();
        var app = "browser";
        var path = "buffer";
        var filename = "sliderBuffer.json";
        var sCount = $('#slider ul li').length;
        fFrBuffer = typeof fFrBuffer === "undefined" ? false : fFrBuffer;
        var imgs;
        if (fFrBuffer) {
            var json = BrowserAPI.readFile(app, path, filename);
            if (!json)
                imgs = CBrowser.getNewImages();
            else {
//                console.log(json.length);
                var tmp = JSON.parse(json);
                imgs = tmp.imgs;
                if (imgs.length <= 0)
                    imgs = CBrowser.getNewImages();
            }
        } else {
            cbh = typeof cbh === "undefined" ? Number(BrowserAPI.getBlockCount()) : cbh;
            var pbh = Number($('#cblc').html());
            if (pbh === cbh && sCount >= 5)
                return;
//            console.log(pbh + "   " + cbh);
            imgs = CBrowser.getNewImages();

        }
        if (imgs.length > 0) {
//            console.log(imgs);
            CBrowser.addSlideImage(imgs);
            var tmp = {imgs: imgs};
            var jtmp = JSON.stringify(tmp);
            var tmp2 = JSON.parse(jtmp);
            BrowserAPI.writeFile(app, path, filename, jtmp);
            this.prepareSlider();
        }
        $("a").click(function () {
            CBrowser.regLink($(this));
        });
    };
    this.getNewImages = function () {
        var ctts = BrowserAPI.getImages(parseInt(BrowserAPI.getBlockCount()), 100, false, 10); // todo change to about 10
        var r = [];
        for (var i in ctts) {
            var cttParsed = this.parseCtt(ctts[i]);
            if (typeof cttParsed.ftype === "undefined")
                continue;
            if ($.inArray(cttParsed.ftype, CONTENT_FILE_TYPE.image) < 0)
                continue;
            r.push(cttParsed);
        }
        console.log(r.length);
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
//        console.log(idiv.html());
        return idiv.html();
    };
    this.addSlideImage = function (imgs) {
        $("#slider ul").html("");
        for (var i in imgs)
            $("#slider ul").append(this.createSliderImage(imgs[i]));
    };
    this.notifyBlock = function (b) {
        $('#cblc').html(b.blockHeight);
        if (currentTab === "br-hot-btn")
            $('#getnew-btn').addClass("hide");
        $('#getnew-btn').removeClass("hide");
//        this.refreshImages(b.blockHeight);
//        var imgs = CBrowser.getNewImages();  // need improvement
//        CBrowser.addSlideImage(imgs);
    };
    this.parseCtt = function (ctt) {
        var r = {};
        r.text = null;
        r.fdata = null;
        r.ldata = null;
        r.ltext = null;
        r.ltype = null;
        r.selflink = typeof ctt.link === "undefined" ? null : ctt.link;

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
        } else if (icc === "CC_LINK_TYPE_BLOCKCHAIN") {
            r.ltype = "BLOCKCHAIN";
        } else if (icc === "CC_NAME") {
            r.name = base64.decode(cCtt);
        } else if (icc === "CC_FILE_TYPESTRING") {
            r.ftype = base64.decode(cCtt);
        } else if (icc === "CC_FILE") {
            r.fdata = cCtt;
        } else if (pcc === "CC_P" || pcc === "CC_FILE_P" || pcc === "CC_TEXT_P" || pcc === "CC_LINK_P") {
            for (var i in c.content) {
                var icc = c.content[i].cc_name;
                var cCtt = c.content[i].content;
                if (icc === "CC_FILE_P" || icc === "CC_TEXT_P" || icc === "CC_LINK_P") {
                    var tmp = this.parseCtt(c.content[i]);
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
        }
        if (!r.lname)
            r.lname = r.ldata;
        return r;
    };
    this.setFollow = function (str2fl, type) {
        type = (typeof type === "undefined") ? "id" : type;
        var myid = BrowserAPI.getAccountID();
        var fkey = "followed-" + type;
        var followedstr = BrowserAPI.getConf("browser", myid, "", fkey);
//        console.log(followed);
        if (!followedstr) {
            if (BrowserAPI.setConf("browser", myid, "", fkey, JSON.stringify([str2fl])))
                return [str2fl];
        }
        else {
            var followed = JSON.parse(followedstr);
            for (var i in followed) {
                if (followed[i] === str2fl)
                    return followed;
            }
            followed.push(str2fl);
            if (BrowserAPI.setConf("browser", myid, "", fkey, JSON.stringify(followed)))
                return followed;
        }
        return false;
    };
    this.setUnfollow = function (str2unfl, type) {
        type = (typeof type === "undefined") ? "id" : type;
        var myid = BrowserAPI.getAccountID();
        if (type === "domain") {
            var domain = BrowserAPI.getDomainInfo(str2unfl);
//                console.log(domain);
            if (typeof domain.forward !== "undefined") {
                if (domain.forward.linkType === "ID") {
                    var fid = domain.forward.target;
                    this.setUnfollow(fid);
                }
            }
        }
        var fkey = "followed-" + type;
        var followedstr = BrowserAPI.getConf("browser", myid, "", fkey);
        if (!followedstr)
            return false;
        else {
            var followed = JSON.parse(followedstr);
//            followed.push(id);
            for (var i in followed) {
                if (followed[i] === str2unfl)
                    followed.splice(i, 1);
            }
            if (BrowserAPI.setConf("browser", myid, "", fkey, JSON.stringify(followed)))
                return true;
        }
        return false;
    };
    this.getFollowed = function (type) {
        type = (typeof type === "undefined") ? "all" : type;
        var myid = BrowserAPI.getAccountID();
        var r = [];
        console.log(type);
        if (type === "all") {
            var fkey = "followed-id";
            var followedstr = BrowserAPI.getConf("browser", myid, "", fkey);
            if (followedstr)
                r = JSON.parse(followedstr);
            var d = this.getFollowed("domain");
            if (typeof d !== "undefined")
                r = r.concat(d);
        } else if (type === "id") {
            var fkey = "followed-" + type;
            var followedstr = BrowserAPI.getConf("browser", myid, "", fkey);
            r = JSON.parse(followedstr);

        } else if (type === "domain") {
            var fkey = "followed-" + type;
            var domains = JSON.parse(BrowserAPI.getConf("browser", myid, "", fkey));
            for (var i in domains) {
                var domain = BrowserAPI.getDomainInfo(domains[i]);
//                console.log(domain);
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
        for (var id in this.sliderId) {
            clearInterval(this.sliderId[id]);
        }
        this.sliderId = [];
        var sliderInterval = 3000;
        var sliderAuto = setInterval(function () {
            moveRight();
        }, sliderInterval);
        this.sliderId.push(sliderAuto);
        var slideCount = $('#slider ul li').length;
        var slideWidth = $('#slider ul li').width();
        var slideHeight = $('#slider ul li').height();
        var sliderUlWidth = slideCount * slideWidth;
        $('#slider').css({width: slideWidth, height: slideHeight});
        $('#slider ul').css({width: sliderUlWidth, marginLeft: -slideWidth});
        $('#slider ul li:last-child').prependTo('#slider ul');
        $('#slider').mouseover(function () {
            clearInterval(sliderAuto);
        }).mouseleave(function () {
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
        var id = CUtil.getGet("id");
        var link = CUtil.getGet("link");
//        console.log(id);
//        console.log(link);
        if (id !== null)
            return "homepage";
        else if (link !== null)
            return "link";
        else
            return "browser";
    };
    this.getBalanceLevel = function (balance) {
        var l = 0;
        var b;
        console.log(balance);
        if (typeof balance === "number")
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
    this.getBalanceHtml = function (bl, number) {
        number = typeof number === "undefined" ? false : number;
        if (bl == 0)
            return "";
        var ml = Math.min(Math.floor((bl - 1) / 4), 4);
        var sl = bl - ml * 4;
        var p = $("<div></div>");
        var div = $("<div></div>");
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
    this.viewLink = function (link) {
        var url = "fai:browser/?link=" + link;
        BrowserAPI.goToCustomPage(url);
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
        }

    };
    this.prepareNotice = function (page) {
        var nDiv = $("#main-notices-container-tpl").clone(true, true);
        nDiv.attr("id", "main-notices-container").removeClass("hide");
//        console.log(nDiv);
        switch (page) {
            case "homepage":
            case "link":
                $(".column2").prepend(nDiv);
                break;
            case "browser":
            default:
                $("#shdr").prepend(nDiv);
                break;
        }
    };
    this.refreshFollowList = function (flist) {
        $("#list").html("");
        for (var i in flist) {
            var url = "fai:browser/?id=" + flist[i];
            var f = $("<a />").html(flist[i]).attr("href", url).click(function () {
                BrowserAPI.goToCustomPage(url);
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
            console.log($(this).val());
            if ($(this).val() !== "")
                r.push($(this).val());
        });
        return r;
    };
    this.handleFiles = function (files, type) {
        for (var i = 0, f; f = files[i]; i++) {
            this.handleFile(f, type);
        }
    };
    this.handleFile = function (f, type) {
        type = typeof type === "undefined" ? "simple" : type;
        console.log(type);
        var largeFileLimit = 1000000000;
        var perblockLimit = 1000000;
        if (f.size > perblockLimit && type !== "large") {
            CBrowser.showNotice(TR('The maximum file size is 1MB'));
            return;
        } else if (f.size > largeFileLimit && type === "large") {
            CBrowser.showNotice(TR('The maximum file size is 1GB'));
            return;
        }
        var fDirect = type === "batch" ? true : false;
        if (f.name && f.size <= perblockLimit) {
            var r = new FileReader();
            r.readAsDataURL(f);
            r.onload = (function (f, fDirect) {
                return function (e) {
                    var raw = e.target.result;
                    var nf = CUtil.decodeDataUrl(raw);
                    var hash = BrowserAPI.getHash(nf.data);
                    if (hash === bufferedFile.hash)
                        return;
                    nf.name = f.name;
                    var cttH = BrowserAPI.createFileContent(nf);
                    if (fDirect) {
                        var feer = BrowserAPI.getFeeRate(0.15);
                        var r = BrowserAPI.createTxByContent(cttH, feer);
                        console.log(r);
                        return;
                    }
                    var ctt = BrowserAPI.getContentByString(cttH.hex);
                    ctt.poster = {id: BrowserAPI.getAccountID()};
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
            })(f, fDirect);
        } else if (f.name && perblockLimit < f.size <= largeFileLimit) {
            var maxFilePartSize = 1000000;
            var parts = Math.ceil(f.size / maxFilePartSize);
//            console.log(parts);
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
//                console.log(i * maxFilePartSize + "    --   " + (i * maxFilePartSize + size));
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
                        var cttH = BrowserAPI.createFilePartContent(nf);
                        var feer = BrowserAPI.getFeeRate(0.15);
                        console.log(feer);
                        var r = BrowserAPI.createTxByContent(cttH, feer);
                        if (r.success)
                            var rdiv = fp.name + " part " + nf.nPart + " txid:nvout --- " + r.success + ":0 <br />"
                        else
                            var rdiv = fp.name + " part " + nf.nPart + "failed " + r.error + "<br />";
                        $("body").append(rdiv);
                        i++;
                        console.log(i);
                        a(i);
                    };
                })(fp, i);
            };
            a(0);
        }

    };
    this.handleContent = function (ctt) {
        ctt.link = "";
//        if (!CBrowser.addContent(ctt, CONTENT_TYPE_MINE, true))  // preview on the right
//            return false;
        $("#confirmpub").removeAttr('disabled');
        var feer = $("#input-feerate").val() > 0 ? $("#input-feerate").val() * COIN / K : 0;
        var deposit = $("#promctt-value").val() > 0 ? $("#promctt-value").val() * COIN : 0;
//        console.log(deposit);
        var locktime = Math.ceil($("#promctt-date").datepicker("getDate") === null ? 0 : $("#promctt-date").datepicker("getDate").getTime() / 1000 + 86400); // add 24 hour
        var locktime = deposit > 0 ? (locktime > 0 ? locktime : 0) : 0;
//        console.log(deposit);
//        console.log(locktime);
        var toId = $("#pubto").find("input[type='text']").val();
        if (toId)
            deposit = locktime = 0;
        var r = BrowserAPI.createTxByContent(ctt, feer, toId, deposit, locktime);
//        console.log(r);
        if (typeof r !== "undefined") {
            if (r.success.length > 0)
                this.clear();
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
        console.log('length');
        console.log(input.files.length);
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
//        console.log(evt);
//        console.log(type);
        evt.stopPropagation();
        evt.preventDefault();
//        console.log('length');
//        console.log(evt.dataTransfer.files.length);

        if (evt.dataTransfer.files.length > 1 && type !== "batch") {
            alert(TR('Only one file is allowed per time'));
            return;
        }
        CPublisher.handleFiles(evt.dataTransfer.files, type);
        return;
    };
    this.createTextContent = function (t) {
        var ctthex = BrowserAPI.createTextContent(t);
        var ctt = BrowserAPI.getContentByString(ctthex.hex);
        ctt.poster = {id: BrowserAPI.getAccountID()};
        ctt.hex = ctthex.hex;
        return ctt;
    };
    this.createLinkContent = function (l) {
        var ctthex = BrowserAPI.createLinkContent(l);
//        console.log(ctthex);
        var ctt = BrowserAPI.getContentByString(ctthex.hex);
        ctt.poster = {id: BrowserAPI.getAccountID()};
        ctt.hex = ctthex.hex;
        return ctt;
    };
    this.createTagContent = function (tags) {
        var ctthex = "";
        for (var i in tags) {
//            console.log(tags[i]);
            var tgcttH = BrowserAPI.createTagContent(tags[i]);
            ctthex += tgcttH.hex;
//            console.log(tgcttH);

        }
        var ctt = BrowserAPI.getContentByString(ctthex);
        ctt.hex = ctthex;
        return ctt;
    };
    this.createLangContent = function (lang) {
        var langctt = null;
        if (!lang)
            return null;
        if (lang === "en" || lang === "en_US")
            langctt = BrowserAPI.icall("encodecontentunit", ["CC_LANG_EN", "", 1])
        else if (lang === "zh" || lang === "zh_CN")
            langctt = BrowserAPI.icall("encodecontentunit", ["CC_LANG_ZH", "", 1])
        else
            langctt = BrowserAPI.icall("encodecontentunit", ["CC_LANG", lang, 1])
//        console.log(langctt);
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
            var ctthex = BrowserAPI.createPContent(hex);
            var ctt = BrowserAPI.getContentByString(ctthex.hex);
            ctt.poster = {id: BrowserAPI.getAccountID()};
            ctt.hex = ctthex.hex;
//            console.log(ctt);
            return ctt;
        }
    };
    this.addTagField = function () {
        var tagdiv = $("#pubtag-tpl").clone(true, true);
        var tagId = "tag-" + tagCount;
        var tagChkId = "chktag-" + tagCount;
        tagdiv.removeAttr("id").removeClass("hide").find("input[name='tag']").attr('id', tagId);
        tagdiv.find("input[type='checkbox']").attr('id', tagChkId);
        if ($("#promctt").length === 0)
            $(tagdiv).insertBefore($('#pubbtnh'));
        else
            $(tagdiv).insertBefore($('#promctt'));
        tagCount++;
    };
    this.addPromCttField = function () {
        var promcttdiv = $("#promctt-tpl").clone(true, true);
        promcttdiv.attr("id", "promctt").removeClass("hide");
        $(promcttdiv).insertBefore($('#pubbtnh'));
        var tomorrow = new Date();
        tomorrow.setDate(tomorrow.getDate() + 1);
//        console.log(tomorrow);
        $("#promctt-date").datepicker({
            altFormat: '@',
            dateFormat: 'yy-mm-dd',
            minDate: tomorrow
        });

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
        var pubto = $("#pubto-tpl").clone(true, true);
        pubto.attr("id", "pubto").removeClass("hide");
        $(pubto).insertBefore($('#pubbtnh'));
    };
    this.showPreview = function (file) {
        console.log(file);
        if (file.ctt.content[0].content === "non-standard")
            return;
        var sdiv = $("#standard-tpl").clone(true, true);
        console.log(sdiv);
        sdiv.removeAttr("id");
        sdiv.find(".container").attr("id", file.hash);
        var id = CBrowser.getIdFrContent(file.ctt);
        sdiv.find(".id").find(".text").attr("fullid", id);
        sdiv.find(".id").find(".text").html(CBrowser.getShortPId(id));
        sdiv.find(".linkspan").attr("clink", file.ctt.link);
//        log("before fill");
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
    this.showDetails = function () {
        if ($("#input-feerate").hasClass("visible"))
            return;
        $("#cclang-span").html(TR("tag-" + langCode));
        $("#CC_LANG").attr("lang", langCode);
        $("#pubdfttag").removeClass("hide");
        var feediv = $("#fee-tpl")
        feediv.removeAttr("id").removeClass("hide").find("input[type='text']").addClass("visible");
        $(feediv).insertBefore($('#pubbtnh'));
        if ($("#promctt").length === 0 & $("#pubto").length === 0)
            this.addPromCttField();
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
//        console.log(typeof $("#input-feerate").val());
        var minPK = currentFeeRate.minfeer / COIN * K;
//        console.log(feeRatePK + "   " + len);
        var estfee = feeRatePK * len / K;
        $("#estfee").html("φ" + estfee.toString().substring(0, 8));
        $("#sgstr").html(sgstPK.toString().substring(0, 6) + " φ/KB");
        $("#minr").html(minPK.toString().substring(0, 6) + " φ/KB");
    };
    this.refreshProTag = function () {
        var deposit = Math.floor($("#promctt-value").val() > 0 ? $("#promctt-value").val() : 0);
//        console.log(deposit);
        var id = 0;
        var count = 0;
        $("input[name='tag']").each(function () {
            if (count < deposit) {
                if ($(this).val() === "") {
                    id++;
                    return true; // continue
                } else {
                    var tagChkId = "#chktag-" + id;
                    $(tagChkId).prop('checked', true);
                    count++;
                }
            } else {
                var tagChkId = "#chktag-" + id;
                $(tagChkId).prop('checked', false);
            }
            id++;
        });
    };
    this.addDefaultProUntil = function () {
        if ($("#promctt-date").datepicker("getDate") === null) {
            var tomorrow = new Date();
            tomorrow.setDate(tomorrow.getDate() + 1);
            $("#promctt-date").val($.datepicker.formatDate('yy-mm-dd', tomorrow));
        }
    };
    this.shareLink = function (link) {
        $('#pubbtnh').show();
        if (!$("#addlink").attr("disabled")) {
            this.addLinkField();
            $("#addlink").attr('disabled', true);
        }
        $("#input-link").val(link);
    };
    this.shareId = function (id) {
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

    this.commentLink = function (link, id) {
        this.shareLink(link);
        if ($("#promctt").length > 0)
            $("#promctt").remove();
        if ($("#pubto").length <= 0)
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
        console.log("call clear");
        $("#pubpreview").addClass("hide");
        $("#theText").val("");
        $("#promctt-value").val("");
        $("#promctt-date").val("");
        $(".input-tag").val("");
        $(".pubtf input[type='checkbox']").prop("checked", false);
        $("#input-link-name").val("");
        $("#input-link").val("");
        $("#input-feerate").val("");
        if ($("#pubto").length > 0)
            this.togglePromCtt();
    };
    this.regFeeRate = function () {
        currentFeeRate.sgstfeer = BrowserAPI.getFeeRate(0.15);
        currentFeeRate.minfeer = BrowserAPI.getFeeRate();
    };
    this.pubSimpleAction = function () {
//        $("#mainframe").html("<div class=")
        $("#mainframe").html($("#maininput-tpl").html()).attr("id", "maininput");
        $("#maininput").html($("#maininput-tpl").html());
        console.log("m1");
        prepareSimplePub();
        prepareStdTpl();
    };
    this.pubPkgAction = function () {
        $("#mainframe").html($("#maininput-tpl").html()).attr("id", "maininput");
        var pkgdiv = $("#pkginput-tpl").clone(true, true);
        $("#maininput").html(pkgdiv.removeClass("hide").attr("id", "pkginput"));
        preparePkgPub();
//        prepareStdTpl();
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