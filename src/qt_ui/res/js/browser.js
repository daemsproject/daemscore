var CONTENT_TYPE_FEED = 0;
var CONTENT_TYPE_FOLLOW = 1;
var CONTENT_TYPE_MINE = 2;

var CBrowser = new function () {
    var CBrowser = this;
    this.showFullId = function (div, fullId) {
        div.find("a.text").html(fullId.substring(0, 25) + "...");
        div.find("li").find("a").css('display', 'inline-block');
    };
    this.hideFullId = function (div, fullId) {
        div.find("a.text").html(fullId.substring(0, 10) + "...");
        div.find("li").find("a").hide();
    };
    this.toggleFullId = function (div) {
        var fullId = div.find("a.text").attr("fullid");
        if (fullId.substring(0, 25) + "..." === div.find("a.text").html())
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
    this.toggleLink = function (div) {
        var cl = div.parent().parent().find(".linkspan");
        if (cl.html() === "")
            cl.html(cl.attr("clink"));
        else
            cl.html("");
    };

    this.shortenId = function (id) {
        return id.substr(0, 10) + "...";
    };
    this.addContent = function (ctt, fType, fPos) {
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
                sdiv = $("#followstd").clone(true, true);
                break;
        }
        sdiv.removeAttr("id");
        sdiv.find(".id").find(".text").attr("fullid", ctt.poster[0]);
        sdiv.find(".id").find(".text").html(this.shortenId(ctt.poster[0]));
        sdiv.find(".linkspan").attr("clink", ctt.link);

        if (this.isContentImage(ctt))
            sdiv.find(".ctt").html(this.createImgHtml(ctt.link, this.getFileContentFrJson(ctt)));
        else if (this.isContentVideo(ctt))
            sdiv.find(".ctt").html(this.createVideoHtml(ctt.link, this.getFileContentFrJson(ctt)));
        else if (this.isContentAudio(ctt))
            sdiv.find(".ctt").html(this.createAudioHtml(ctt.link, this.getFileContentFrJson(ctt)));
        else if (this.isContentText(ctt))
            sdiv.find(".ctt").html(atob(ctt.content[0].content[0].content));
        else
            return;
        fPos ? $("#mainframe").prepend(sdiv.children()) : $("#mainframe").append(sdiv.children());
    };
    this.isContentImage = function (ctt) {
        return ctt.content[0].cc_name === "CC_FILE_P" &&
                ctt.content[0].content[1].cc_name === "CC_FILE_TYPESTRING" &&
                ctt.content[0].content[1].content === btoa("image/jpeg");
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
        var ctts = this.getNewContents();
        for (k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FEED, false);
        }
    };
    this.refreshOld = function () {
        var ctts = this.getOldContents();
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
        var ctts = this.getNewContents(flist);
        for (k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FEED, false);
        }
    };
    this.refreshOldFollowed = function () {
        var flist = BrowserAPI.getFollowed();
        var ctts = this.getOldContents(flist);
        for (k in ctts) {
            this.addContent(ctts[k], CONTENT_TYPE_FEED, false);
        }
    };
    this.getNewContents = function (addrs) {
        addrs = typeof addrs !== 'undefined' ? addrs : [];
        var rc = 3;
        var lbh = parseInt(BrowserAPI.getBlockCount());
        var bh = lbh;
        bh -= 10;
        var ctts = BrowserAPI.getContents(bh, 10, false, addrs);
        bh -= 10;
        while (ctts.length < rc && bh > 0) {
            var tmp = BrowserAPI.getContents(bh, 10, false, addrs);
            ctts = ctts.concat(tmp);
            bh -= 10;

        }
        blkDisp = [bh + 10, lbh];
        return ctts;
    };
    this.getOldContents = function (addrs) {
        addrs = typeof addrs !== 'undefined' ? addrs : [];
        var rc = 3;
        var lbh = blkDisp[0];
        var bh = lbh;
        bh -= 10;
        var ctts = BrowserAPI.getContents(bh, 10, false, addrs);
        bh -= 10;
        while (ctts.length < rc && bh > 0) {
            var tmp = BrowserAPI.getContents(bh, 10, false, addrs);
            ctts = ctts.concat(tmp);
            bh -= 10;

        }
//        console.log(blkDisp);
        blkDisp = [bh + 10, lbh];
        return ctts;
    };
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
        }
//        blkDisp = [];


    };
    this.newAction = function () {
        this.refreshNew();
        this.refreshOld();
    };
    this.followedAction = function () {
        this.refreshNewFollowed();
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
                this.newAction();
                break;
            case "br-followed-btn":
                this.refreshOldFollowed();
                break;
        }
//        console.log(tabid);
    };
};
var blkDisp;
var fllDisp;
$(document).ready(function () {

    CBrowser.newAction();

    $("#refresh-btn").click(function () {
        CBrowser.refreshNew();
    });
    $("#refreshold-btn").click(function () {
        CBrowser.refreshOld();
    });
    $(".tabbar").children("li").children("a").click(function () {
        CBrowser.switchTab($(this).attr("id"));
    });
    $(".id").find("a.text").click(function () {
        CBrowser.toggleFullId($(this).parent());
    });
    $(".id-follow-btn").click(function () {
        var id = $(this).parent().parent().find(".text").attr("fullid");
        var feedback = BrowserAPI.setFollow(id);
        for (k in feedback) {
            if (feedback[k] == id) {
                CBrowser.showNotice("Successful");
                return;
            }
        }
        CBrowser.showNotice("Failed");
        console.log(feedback);
    });
    $(".id-share-btn").click(function () {
        alert("To Do");
    });
    $(".brctt").find("a.shrt").click(function () {
        CBrowser.toggleCmt($(this).parent());
    });
    $(".ctt").click(function () {
        CBrowser.showFullImg($(this));
    });
    $(".ctt-link-btn").click(function () {
        CBrowser.toggleLink($(this));
    });


    $("#fullImage").click(function () {
        $(this).html("");
    });
    $("#test-btn").click(function () {
        console.log(BrowserAPI.getFollowed());
    });
    $(window).scroll(function () {
        if ($(window).scrollTop() + $(window).height() == $(document).height()) {
            CBrowser.bottomAction();
        }
    });
});
