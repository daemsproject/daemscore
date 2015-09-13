var CHelp = new function () {
    CHelp = this;
    this.HELP = {};
    this.helpFile = '../json/help_en.json';
    this.keyWords = ["name", "text", "class"];
    this.getJson = function (path) {
        path = typeof path === "undefined" ? this.helpFile : path;
        var t;
        $.ajax({
            url: path,
            async: false,
            dataType: 'json',
            success: function (data) {
                t = data;
            },
            error: function () {
                console.log("Parse help json error");
            }
        });
        this.HELP = t;
//        console.log(this.HELP);
    };
    this.isParentOjb = function (obj, fIncludeArray) {
        fIncludeArray = typeof fIncludeArray === "undefined" ? false : fIncludeArray;
        for (var i in obj) {
            if (typeof obj[i] === "object") {
//                console.log(obj[i]);
//                console.log(Object.prototype.toString.call(obj[i]));
                if (fIncludeArray)
                    return true;
                else if (Object.prototype.toString.call(obj[i]) !== "[object Array]")
                    return true;
                else
                    continue;
            }
        }
        return false;
    }
    this.renderHelp = function () {
        console.log(this.HELP);
        for (var i in this.HELP) {
            var menu = $("#dropdown-menu-tpl").clone(true, true).removeAttr("id");
            menu.find("a").find(".name").attr("tr", this.HELP[i].name);
            if (this.HELP[i].class)
                menu.find("a").addClass(this.HELP[i].class);
            if (CUtil.hasChild(this.HELP[i])) {
//                console.log(i);
                for (var k in this.HELP[i]) {
//                    console.log(k);
                    if (typeof this.HELP[i][k] === "object")
//                        console.log(this.HELP[i][k]);
//                    console.log(this.HELP[i][k]);
//                    console.log(this.isParentOjb(this.HELP[i][k]));

                        menu.find("ul.multi-level").append(this.renderSubMenu(this.HELP[i][k], k));
                }
            }
            $("#drop-menu").append(menu.children());
        }
    };
    this.renderSubMenu = function (obj, id) {
        var m = $("#submenu-tpl").clone(true, true).removeAttr("id");
        m.find("a").attr("tr", obj.name).attr("id", id).addClass("goto");
        if (this.isParentOjb(obj)) {
            m.find("li").addClass("dropdown-submenu");
            for (var i in obj) {
                if (typeof obj[i] === "object") {
//                    console.log(obj[i], i)
//                    console.log(Object.prototype.toString.call(obj[i]) !== "[object Array]");
                    m.find("ul").append(this.renderSubMenu(obj[i], i));
                }
            }
        } else
            m.find("ul").remove();
        return m.children();
    };
//    this.getHelpLink = function (id) {
//        return "?help=" + id;
//    };
    this.getHelpId = function () {
        var id = CUtil.getGet("hid");
        return id;
    };
    this.renderHelpContent = function (id) {
        id = typeof id === "undefined" ? CHelp.getHelpId() : id;
//        console.log(id);
        var hctt = this.getHelpContent(id);
        var r = $("<div />");
        if (CUtil.isArray(hctt)) {
            for (var i in hctt) {
                var p = $("<p />").html(hctt[i]);
                r.append(p);
            }
        } else
            r = hctt;
        $("#drop-menu-content").html(r);
    };
    this.getHelpContent = function (id) {
        for (var i in this.HELP) {
            if (i === id)
                return this.HELP[i].text;
            var r = this.getHelpContentFrObj(id, this.HELP[i]);
//            console.log(i);console.log(id);
            if (r.length > 0)
                return r;
        }
        return [];
    };
    this.getHelpContentFrObj = function (id, obj) {
        if (CUtil.hasChild(obj)) {
            for (var i in obj) {
                if (i === id)
                    return obj[i].text;
                if (typeof obj[i] === "object") {
                    var r = this.getHelpContentFrObj(id, obj[i]);
                    if (r.length > 0)
                        return r;
                }
            }
        }
        return "";
    };
};