var CONTENT_TYPE_FEED = 0;
var CONTENT_TYPE_FOLLOW = 1;
var CONTENT_TYPE_MINE = 2;

var CBrowser = new function () {
    var CBrowser = this;
    this.getShortPId = function (fullId) {
        return fullId.substr(0, 10) + "..." + fullId.substr(fullId.length - 2);
    };
    this.getLongPId = function (fullId) {
        return fullId.substr(0, 25) + "..." + fullId.substr(fullId.length - 2);
    };
    this.showFullId = function (div, fullId) {
        div.find("a.text").html(this.getLongPId(fullId));
        div.find("li").find("a").css('display', 'inline-block');
    };
    this.hideFullId = function (div, fullId) {
        div.find("a.text").html(this.getShortPId(fullId));
        div.find("li").find("a").hide();
    };
    this.toggleFullId = function (div) {
        var fullId = div.find("a.text").attr("fullid");
        if (this.getLongPId(fullId) === div.find("a.text").html())
            this.hideFullId(div, fullId);
        else
            this.showFullId(div, fullId);
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
    this.getImage = function (clink) {
        var cj = (BrowserAPI.getContentByLink(clink));
        r = this.getFileContentFrJson(cj);
        return this.createImgHtml(clink, r);

    };
    this.createImgHtml = function (clink, imgB64Data, type) {
        type = typeof type !== 'undefined' ? type : "image/jpeg";
        var idiv = $("#image-tpl").clone(true, true);
        idiv.find("img").attr("id", CLink.setString(clink).toHtmlId());
        idiv.find("img").attr("type", type);
        idiv.find("img").attr("src", "data:" + type + ";base64," + imgB64Data);
        return idiv.html();
    };
    this.createVideoHtml = function (clink, vdoB64Data, type) {
        type = typeof type !== 'undefined' ? type : "video/mp4";
        var vdiv = $("#video-tpl").clone(true, true);
        vdiv.find("video").attr("id", CLink.setString(clink).toHtmlId());
        vdiv.find("source").attr("type", type);
        vdiv.find("source").attr("src", "data:" + type + ";base64," + vdoB64Data);
        return vdiv.html();
    };
    this.createAudioHtml = function (clink, adoB64Data, type) {
        type = typeof type !== 'undefined' ? type : "audio/mpeg";
        var vdiv = $("#video-tpl").clone(true, true);
        vdiv.find("video").attr("id", CLink.setString(clink).toHtmlId());
        vdiv.find("source").attr("type", type);
        vdiv.find("source").attr("src", "data:" + type + ";base64," + adoB64Data);
        return vdiv.html();
    };
    this.toggleCmt = function (div) {
        var s = div.find("a.shrt").hasClass("short");
        if (s) {
            div.find("li.linkspan").html(this.getLinkA(div.find("li.linkspan").attr("clink")));
            div.find("li").find("a").show();
            div.find("a.shrt").removeClass("short");

        } else {
            div.find("li").find("a").hide();
            div.find("a.shrt").addClass("short");
        }
    };
    this.showFullImg = function (div) {
        if (div.find("img").length !== 0) {
            $("#fullImage").html(div.html());
            $("#fullImage").find("img").removeClass("brimg");
        }
    };
    this.getLinkA = function (l) {
        var ldiv = $("#linka").clone(true, true);
        ldiv.find("a").html(l).show();
        console.log(ldiv.html());
        return ldiv.html();
    };
    this.addContent = function (ctt, fType, fPos) {
//        console.log(ctt);
        fPos = typeof fPos !== 'undefined' ? fPos : true;
        fType = typeof fType !== 'undefined' ? fType : CONTENT_TYPE_FEED;
        if (ctt.content[0].content === "non-standard")
            return;
        var sdiv
        switch (fType) {
            case CONTENT_TYPE_FEED:
                sdiv = $("#standard").clone(true, true);
                break;
            case CONTENT_TYPE_FOLLOW:
            case CONTENT_TYPE_MINE:
                sdiv = $("#followstd").clone(true, true);
                break;
        }
        sdiv.removeAttr("id");
        sdiv.find(".id").find(".text").attr("fullid", ctt.poster[0]);
        sdiv.find(".id").find(".text").html(this.getShortPId(ctt.poster[0]));
        sdiv.find(".linkspan").attr("clink", ctt.link);

        if (this.isContentImage(ctt))
            sdiv.find(".ctt").html(this.createImgHtml(ctt.link, this.getFileContentFrJson(ctt)));
        else if (this.isContentVideo(ctt))
            sdiv.find(".ctt").html(this.createVideoHtml(ctt.link, this.getFileContentFrJson(ctt)));
        else if (this.isContentAudio(ctt))
            sdiv.find(".ctt").html(this.createAudioHtml(ctt.link, this.getFileContentFrJson(ctt)));
        else if (this.isContentText(ctt))
            sdiv.find(".ctt").html(base64.decode(ctt.content[0].content[0].content));
        else {
            console.log("err addcontent");
            return false;
        }
//        console.log(sdiv.html());
        fPos ? $("#mainframe").prepend(sdiv.children()) : $("#mainframe").append(sdiv.children());
        return true;
    };
    this.isContentImage = function (ctt) {
        return ctt.content[0].cc_name === "CC_FILE_P" &&
                ctt.content[0].content[1].cc_name === "CC_FILE_TYPESTRING" &&
                (ctt.content[0].content[1].content === btoa("image/jpeg") || ctt.content[0].content[1].content === btoa("image/png"));
    };
    this.isContentVideo = function (ctt) {
        return ctt.content[0].cc_name === "CC_FILE_P" &&
                ctt.content[0].content[1].cc_name === "CC_FILE_TYPESTRING" &&
                ctt.content[0].content[1].content === btoa("video/mp4");
    };
    this.isContentAudio = function (ctt) {
        return ctt.content[0].cc_name === "CC_FILE_P" &&
                ctt.content[0].content[1].cc_name === "CC_FILE_TYPESTRING" &&
                ctt.content[0].content[1].content === btoa("audio/mpeg");
    };
    this.isContentText = function (ctt) {
        return ctt.content[0].cc_name === "CC_TEXT_P" &&
                ctt.content[0].content[0].cc_name === "CC_TEXT" &&
                ctt.content[0].content[0].content.length !== 0;
    };

    this.refreshNew = function () {
        var ctts = this.getNewContents("new");
        for (k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FEED, false);
        }
    };
    this.refreshOld = function () {
        var ctts = this.getOldContents("new");
        for (k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FEED, false);
        }
    };
    this.refreshNewFollowed = function () {
        var flist = BrowserAPI.getFollowed();
        if (flist.length == 0) {
            this.showNotice("You need to follow someone first");
            return;
        }
        var ctts = this.getNewContents("fll", flist);
        for (k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FOLLOW, false);
        }
    };
    this.refreshNewMypage = function () {
        var myid = [BrowserAPI.getAccountID()];
        if (myid.length == 0) {
            this.showNotice("You need to register ID first");
            return;
        }
        var ctts = this.getNewContents("myp", myid);
        for (k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_MINE, false);
        }
    };
    this.refreshOldFollowed = function () {
        var flist = BrowserAPI.getFollowed();
        var ctts = this.getOldContents("fll", flist);
        for (k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FOLLOW, false);
        }
    };
    this.refreshOldMypage = function () {
        var myid = [BrowserAPI.getAccountID()];
        var ctts = this.getOldContents("myp", myid);
        for (k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_MINE, false);
        }
    };
    this.getNewContents = function (page, addrs) {
        return this.getContents(true, page, addrs);
    };
    this.getOldContents = function (page, addrs) {
        return this.getContents(false, page, addrs);
    };
    this.getContents = function (fNewOld, page, addrs) {
        addrs = typeof addrs !== 'undefined' ? addrs : [];
        fNewOld = typeof fNewOld !== 'undefined' ? fNewOld : true;
        page = typeof page !== 'undefined' ? page : "new";
        var rc = 3;
        var sbh; // start block height
        var lbh; // last block height
        switch (page) {
            case "new":
                sbh = fNewOld ? parseInt(BrowserAPI.getBlockCount()) : newDisp[0];
                lbh = fNewOld ? sbh : newDisp[1];
                break;
            case "fll":
                sbh = fNewOld ? parseInt(BrowserAPI.getBlockCount()) : fllDisp[0];
                lbh = fNewOld ? sbh : fllDisp[1];
                break;
            case "myp":
                sbh = fNewOld ? parseInt(BrowserAPI.getBlockCount()) : mypDisp[0];
                lbh = fNewOld ? sbh : mypDisp[1];
                break;
        }
//        console.log(fNewOld + " " + sbh);
        var cbh = sbh; // current block height
        var ctts = [];
        while (ctts.length < rc && cbh > 0) {
            var tmp = BrowserAPI.getContents(cbh, 10, false, addrs);
            ctts = ctts.concat(tmp);
            cbh -= 10;

        }
//        cbh += 10;
//        console.log(fNewOld + " " + cbh);
        switch (page) {
            case "new":
                var fbh = newDisp.length > 0 ? newDisp[0] : cbh;
//                console.log(fbh + " " + fNewOld + " " + cbh);
                newDisp = fNewOld ? [fbh, lbh] : [cbh, lbh];
                break;
            case "fll":
                var fbh = fllDisp.length > 0 ? fllDisp[0] : cbh;
                fllDisp = fNewOld ? [fbh, lbh] : [cbh, lbh];
                break;
            case "myp":
                var fbh = mypDisp.length > 0 ? mypDisp[0] : cbh;
                mypDisp = fNewOld ? [fbh, lbh] : [cbh, lbh];
                break;
        }
//        console.log(newDisp);
//        console.log(fllDisp);
//        console.log(mypDisp);

        return ctts;

    }
    this.switchTab = function (tabid) {
//        console.log(tabid);
        if ($("#" + tabid).parent().hasClass("active"))
            return;
        $("#" + tabid).parent().parent().children("li").removeClass("active");
        $("#" + tabid).parent().addClass("active");
        $("#mainframe").children(".container").remove();
        $("#mainframe").children("hr").remove();
        switch (tabid) {
            case "br-new-btn":
                this.newAction();
                break;
            case "br-followed-btn":
                this.followedAction();
                break;
            case "br-mypage-btn":
                this.mypageAction();
                break;
        }
//        newDisp = [];


    };
    this.newAction = function () {
        newDisp = [];
        this.refreshNew();
//        this.refreshOld();
    };
    this.followedAction = function () {
        this.refreshNewFollowed();
    };
    this.mypageAction = function () {
        this.refreshNewMypage();
    };
    this.showNotice = function (n) {
        $("#notices").html(n).show();
        $("#notices").delay(2000).hide(0);
    };
    this.bottomAction = function () {
        var tabid = $(".tabbar").children("li.active").children("a").attr("id");
//        var page = this.findPage();
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
//        console.log(tabid);
//        console.log(newDisp);
    };
};

var CPublisher = new function () {
    var CPublisher = this;

    this.handleFiles = function (files) {
        for (var i = 0, f; f = files[i]; i++) {
            if (f.size > 1000000) {
                CBrowser.showNotice("The maximum file size is 10MB");
                return;
            }
            if (f.name) {
                var r = new FileReader();
                r.readAsDataURL(f);
                r.onload = (function (f) {
                    return function (e) {
                        var raw = e.target.result;
                        var rctt = CUtil.decodeDataUrl(raw);
                        rctt["filename"] = f.name;
                        console.log("rctt ");
                        console.log(rctt);

                        var ctthex = BrowserAPI.createContentC(rctt.type, rctt);
//                        var start = new Date().getTime();
//                        console.log(start);
                        console.log(ctthex.hex.substr(0, 20) + "...(" + ctthex.hex.length / 2 + " bytes)");
//                        var end = new Date().getTime();
//                        console.log(end);
                        var ctt = BrowserAPI.getContentByString(ctthex.hex);
                        ctt.poster = [BrowserAPI.getAccountID()];
                        ctt.hex = ctthex.hex;
                        CPublisher.handleContent(ctt);
                    };
                })(f);
            }
        }
    };
    this.handleContent = function (ctt) {
        ctt.link = "";
        console.log(ctt);
        if (!CBrowser.addContent(ctt, CONTENT_TYPE_MINE, false))
            return false;
        $("#confirmpub").removeAttr('disabled');
        var rawtx = BrowserAPI.createTxByContent(ctt);
        console.log(rawtx);
    };
    this.handleDragOver = function (evt) {
        evt.stopPropagation();
        evt.preventDefault();
        evt.dataTransfer.dropEffect = 'copy'; // Explicitly show this is a copy.
    };
    this.handleFileInput = function (elemId) {
        var elem = document.getElementById(elemId);
        if (elem && document.createEvent) {
            var evt = document.createEvent("MouseEvents");
            evt.initEvent("click", true, false);
            elem.dispatchEvent(evt);
        }

        input = document.getElementById('theFile');
        this.handleFiles(input.files);
    };
    this.handleFileSelect = function (evt) {
        evt.stopPropagation();
        evt.preventDefault();
        CPublisher.handleFiles(evt.dataTransfer.files);
        return;
    };
    this.handleText = function (t) {
        var ctthex = BrowserAPI.createTextContent(t);
        var ctt = BrowserAPI.getContentByString(ctthex.hex);
        ctt.poster = [BrowserAPI.getAccountID()];
        ctt.hex = ctthex.hex;
        CPublisher.handleContent(ctt);
    };
};