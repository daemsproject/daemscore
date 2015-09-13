var Shop = new function () {
    var i = this;
    var cVisible; //currently visible view    
    var accountID;
    var haveBoundReady = false;
    var pagelen = 100;
    var page = 0;
    var products = [];
    var shopProducts = [];
    var myProducts = [];
    var currentProduct;
    var cart = {};
    function bindInitial() {
        $('.modal').on('show', function () {
            hidePopovers();

            $(this).center();
        }).on('hidden', function () {
            var visible = $('.modal:visible');

            var notices = $('#notices').remove();

            if (visible.length > 0)
                visible.find('.modal-body').prepend(notices);
            else
                $('#main-notices-container').append(notices);

        }).on('shown', function () {
            hidePopovers();

            var self = $(this);
            setTimeout(function () {
                if (self.is(':visible')) {
                    self.find('.modal-body').prepend($('#notices').remove());
                }
            }, 100);

            self.center();
        });
    }
    function hidePopovers() {
        try {
            $('.popover').remove();
        } catch (e) {
        }
    }
    function registerNotifications() {
        console.log("regnotifications");
        var ab = function (a) {
            CPage.updateBalance();
        };
        var ac = function (a) {
            console.log("refresh");
            window.location.href = window.location.href;
        }
        FAI_API.regNotifyTxs(ab, [accountID]);
        FAI_API.regNotifyAccount(ac);
        console.log("regnotifications success");
    }
    this.initAddNew = function ()
    {
        $("#add-new").find('input[name="product-id"]').val(parseInt(Math.random() * 256 * 256 * 256 * 256));
    }
    this.showViewProduct = function (tags) {

        var params = {};
        if (tags)
        {
            if (Object.prototype.toString.call(tags) != '[object Array]')
                tags = [tags];
            params.tags = tags;
        }
        params.offset = page * pagelen;
        params.maxc = pagelen;
        products = FAI_API.searchProducts(params);
        console.log(products);
        if (!products || products.error || products.length == 0) {
            $('#buy-product-list').html("<tr><td>" + TR('no products found, please change search tags and retry') + "</td></tr>");
            return;
        }
        var html = "<tr><th>" + TR('Icon') + "</th><th>" + TR('Product Name') + "</th><th>" + TR('Price') + "</th><th>" + TR('Seller') + "</th><th>" + TR('Action') + "</th></tr>";
        for (var j in products) {
            html += "<tr><td>";
            var p = products[j];
            if (p.icon) {
                html += CPage.createImgHtml(CBrowser.getB64DataFromLink(p.icon));
            }
            html += '</td><td>';
            html += p.name.substr(0, 32);
            html += '</td><td>';
            html += ("φ" + p.price);
            html += '</td><td>';
            if (p.seller) {
                if (p.seller.domain)
                {
                    if (p.seller.domain.alias)
                        html += p.seller.domain.alias;
                    else
                        html += p.seller.domain.domain;
                }
                else if (p.seller.id)
                    html += showID(p.seller.id);
            }
            html += '</td><td>';
            html += '<button class="idbtn light" id="btn-' + p.id + ' title="detail" onclick="{Shop.showDetails(\'' + p.id + '\',true)}">' + TR('Details') + '</button>';
            html += '</td></tr>';
        }
        $('#buy-product-list').html(html);
    }
    this.shopOverView = function () {

        var params = {};
        params.ids = [accountID];
        params.offset = page * pagelen;
        params.maxc = pagelen;
        shopProducts = FAI_API.searchProducts(params);
        console.log(shopProducts);
        if (!shopProducts || shopProducts.error || shopProducts.length == 0) {
            $('#seller-product-list').html("<tr><td>" + TR('no products available now,please publish one') + "</td></tr>");
            return;
        }
        var html = "<tr><th>" + TR('Product ID') + "</th><th>" + TR('Icon') + "</th><th>" + TR('Product Name') + "</th><th>" + TR('Price') + "</th><th>" + TR('Recipient') + "</th><th>" + TR('Expire Time') + "</th><th>" + TR('Action') + "</th></tr>";
        for (var j in shopProducts) {
            var p = shopProducts[j];
            html += "<tr><td>";
            html += p.id;
            html += '</td><td>';
            if (p.icon) {
                html += CPage.createImgHtml(CBrowser.getB64DataFromLink(p.icon));
            }
            html += '</td><td>';
            html += p.name.substr(0, 32);
            html += '</td><td>';
            html += ("φ" + p.price);
            html += '</td><td>';
            html += showID(p.recipient);
            html += '</td><td>';
            if (p.expiretime)
                html += CUtil.dateToString(new Date(p.expiretime * 1000));
            html += '</td><td>';
            html += '<button class="idbtn light" id="btn-' + p.id + ' title="detail" onclick="{Shop.showDetails(\'' + p.id + '\')}">' + TR('Details') + '</button>';
            html += '</td></tr>';
        }
        $('#seller-product-list').html(html);
    }
    this.showSalesRecord = function () {
        var records = FAI_API.getSalesRecord([accountID]);
        if (!records || records.error || records.length == 0) {
            $('#seller-product-list').html("<tr><td>" + TR('no sales record yet,please try some promotions') + "</td></tr>");
            return;
        }
        var html = "<tr><th>" + TR('Time') + "</th><th>" + TR('Product ID') + "</th><th>" + TR('Icon') + "</th><th>" + TR('Product Name') + "</th><th>" + TR('Price') + "</th><th>" + TR('Quantity') + "</th><th>" + TR('Customer') + "</th><th>" + TR('Total') + "</th><th>" + TR('Actual Paid') + "</th></tr>";
        for (var j in records) {
            var r = records[j];
            var total = 0;
            if (r.paymentitems)
            {
                for (var i in r.paymentitems) {
                    var it = r.paymentitems[i];
                    html += "<tr><td>";
                    html += CUtil.dateToString(new Date(r.time * 1000));
                    html += '</td><td>';
                    if (it.productID)
                        html += it.productID;
                    html += '</td><td>';
                    var p;
                    for (var j in shopProducts)
                        if (shopProducts[j].id = it.productID)
                            p = shopProducts[j];
                    if (p && p.icon) {
                        html += CPage.createImgHtml(CBrowser.getB64DataFromLink(p.icon));
                    }
                    html += '</td><td>';
                    if (it.type == "CC_PAYMENT_TYPE_SHIPMENTFEE")
                        html += TR('shipment fee');
                    else if (p && p.name)
                        html += p.name.substr(0, 32);
                    html += '</td><td>';
                    html += ("φ" + it.price);
                    html += '</td><td>';
                    html += it.quantity;
                    html += '</td><td><div class="sid" fullid="' + r.payer.id + '">';
                    if (r.payer.domain)
                        html += r.payer.domain.domain;
                    else
                        html += CUtil.getShortPId(r.payer.id);
                    html += '</div></td><td>';
                    html += ("φ" + it.price * it.quantity);
                    total += it.price * it.quantity;
                    html += '</td><td>';
                    html += '</td></tr>';
                }
            }
            html += "<tr><td>";
            html += CUtil.dateToString(new Date(r.time * 1000));
            html += '</td><td>';
            html += '</td><td>';
            html += '</td><td>';
            html += '</td><td>';
            html += '</td><td>';
            html += '</td><td><div class="sid" fullid="' + r.payer.id + '">';
            if (r.payer.domain)
                html += r.payer.domain.domain;
            else
                html += CUtil.getShortPId(r.payer.id);
            html += '</div></td><td>';
            html += ("φ" + total);
            html += '</td><td>';
            html += ("φ" + r.paidvalue);
            html += '</td></tr>';
        }
        $('#table-sales-record').html(html);
        $(".sid").click(function () {
            Shop.showIdModal(r.payer, "Buyer Details");
        });
    };
    this.showIdModal = function (r, h) {
        var pdiv = $("#poster-tpl").clone(true, true).removeAttr("id");
        var domain = r.domain;
        var id2show = $.isEmptyObject(domain) ? CUtil.getShortPId(r.id) : domain.domain;
        var idtype = $.isEmptyObject(domain) ? "" : "(domain)";
        pdiv.find(".id").find(".text").html(id2show);
        pdiv.find(".id").find(".text").attr("fullid", r.id);
        if (!$.isEmptyObject(domain))
            pdiv.find(".id").find(".text").attr("domain", domain.domain);
        pdiv.find(".id").find(".idtype").html(idtype);
        pdiv.find(".id-follow-btn").parent().remove();
        pdiv.find(".id-unfollow-btn").parent().remove();
        var htext = TR(h);
        $("#modal-id-details").find(".modal-header").find("h3").html(htext);
        $("#modal-id-details").find(".modal-body").html("").append(pdiv.children()).css("min-width", 700);
        $("#modal-id-details").modal({show: true}).center();
    };
    this.showMyOrders = function () {
        var records = FAI_API.getPurchaseRecord([accountID]);
        console.log(records);
        if (!records || records.error || records.length == 0) {
            $('#my-orders').html("<tr><td>" + TR('no purchase records') + "</td></tr>");
            return;
        }
        var html = "<tr><th>" + TR('Time') + "</th><th>" + TR('Product ID') + "</th><th>" + TR('Icon') + "</th><th>" + TR('Product Name') + "</th><th>" + TR('Price') + "</th><th>" + TR('Quantity') + "</th><th>" + TR('Seller') + "</th><th>" + TR('Total') + "</th><th>" + TR('Actual Paid') + "</th><th>" + TR('Action') + "</th></tr>";
        for (var j in records) {
            var r = records[j];
            var total = 0;
            if (r.paymentitems)
            {
                for (var jj in r.paymentitems) {
                    var it = r.paymentitems[jj];
                    html += "<tr><td>";
                    html += CUtil.dateToString(new Date(r.time * 1000));
                    html += '</td><td>';
                    if (it.productID)
                        html += it.productID;
                    html += '</td><td>';

                    var p;
                    if (it.productID) {
                        var fFound = false;
                        for (var k in myProducts)
                            if (myProducts[k]&&myProducts[k].id&&myProducts[k].id == it.productID)
                            {
                                p = myProducts[k];
                                fFound0 = true;
                                break;
                            }
                        if (fFound===false) {
                            p = FAI_API.getProductByLink(it.paytolink);
                            if (p.id)
                                myProducts.push(p);
                        }
                    }
                    if (p && p.icon) {
                        html += CPage.createImgHtml(CBrowser.getB64DataFromLink(p.icon));
                    }
                    html += '</td><td>';
                    if (it.type == "CC_PAYMENT_TYPE_SHIPMENTFEE")
                        html += TR('shipment fee');
                    else if (p && p.name)
                        html += p.name.substr(0, 32);
                    html += '</td><td>';
                    html += ("φ" + it.price);
                    html += '</td><td>';
                    html += it.quantity;
                    html += '</td><td><div class="sid">';
                    if (p && p.seller) {
                        if (p.seller.domain)
                        {
                            if (p.seller.domain.alias)
                                html += p.seller.domain.alias;
                            else
                                html += p.seller.domain.domain;
                        }
                        else if (p.seller.id)
                            html += CUtil.getShortPId(p.seller.id);
                    }
                    html += '</div></td><td>';
                    html += ("φ" + it.price * it.quantity);
                    total += it.price * it.quantity;
                    html += '</td><td>';
                    html += '</td><td>';
                    if (p)
                        html += '<button class="idbtn light" id="btn-' + it.productID + ' title="detail" onclick="{Shop.showDetails(\'' + it.productID + '\',true)}">' + TR('Details') + '</button>';
                    html += '</td></tr>';
                }
            }
            html += "<tr><td>";
            html += CUtil.dateToString(new Date(r.time * 1000));
            html += '</td><td>';
            html += '</td><td>';
            html += '</td><td>';
            html += '</td><td>';
            html += '</td><td>';
            html += '</td><td><div class="sid">';
            if (p && p.seller) {
                if (p.seller.domain)
                {
                    if (p.seller.domain.alias)
                        html += p.seller.domain.alias;
                    else
                        html += p.seller.domain.domain;
                }
                else if (p.seller.id)
                    html += CUtil.getShortPId(p.seller.id);
            }
            html += '</div></td><td>';
            html += ("φ" + total);
            html += '</td><td>';
            html += ("φ" + r.paidvalue);
            html += '</td><td>';
            html += '</td></tr>';
        }
        $('#table-myorders').html(html);
        $(".sid").click(function () {
            Shop.showIdModal(p.seller, "Seller Details");
        });
    }
    this.showDetails = function (id, fTocart) {
        var p;
        for (var j in products)
            if (products[j].id == id) {
                p = products[j];
                currentProduct = p;
            }
        if (!p)
            for (var j in myProducts)
                if (myProducts[j].id == id) {
                    p = myProducts[j];
                    currentProduct = p;
                }
        if (p) {
            var html = "";
            html += "<div>" + TR('Product ID') + ":" + p.id + "</div>";
            if (p.icon) {
                html += '<div>' + CPage.createImgHtml(CBrowser.getB64DataFromLink(p.icon), p.icon, 200, 200) + '</div>';
            }
            html += '<div>' + TR('Product Name') + ':' + p.name + '</div>';
            html += '<div>' + TR('Price') + ':' + ("φ" + p.price) + '</div>';
            if (p.shipmentfee)
                html += '<div>' + TR('shipment fee') + ':' + ("φ" + p.shipmentfee) + '</div>';
            if (p.seller) {
                if (p.seller.domain)
                {
                    if (p.seller.domain.alias)
                        html += '<div>' + TR('Shop') + ':' + p.seller.domain.alias + '</div>';
                    else
                        html += '<div>' + TR('Shop') + ':' + p.seller.domain.domain + '</div>';
                }
                else if (p.seller.id)
                    html += '<div>' + TR('Shop') + ':' + showID(p.seller.id) + '</div>';
            }
            if (p.intro)
                html += '<div><span>' + TR('Introductions') + ':</span>' + p.intro + '</div>';

            var pdiv = CPage.prepareProdDiv();
            pdiv.find(".prd-pchs-btn").remove();
            pdiv.find(".ctt-share-btn").remove();
            pdiv.find(".ctt-cmt-btn").remove();

            pdiv.find(".id-follow-btn").parent().remove();
            pdiv.find(".id-unfollow-btn").parent().remove();
            pdiv = CPage.fillProdDiv(pdiv, p);
            $("#modal-product-details").find(".modal-body").html("").append(pdiv);



            if (fTocart)
                $("#modal-product-details").find("#btn-addcart").show();
            else
                $("#modal-product-details").find("#btn-addcart").hide();
        }

        $("#modal-product-details").modal({show: true}).center();
    }
    this.refreshCart = function () {
        var hasContent = false;
        var html = "<tr><th>" + TR('Icon') + "</th><th>" + TR('Product Name') + "</th><th>" + TR('Seller') + "</th><th>" + TR('Price') + "</th><th>" + TR('Shipment fee') + "</th><th>" + TR('Quantity') + "</th><th>" + TR('SubTotal') + "</th><th></th></tr>";
        for (var j in cart) {
            hasContent = true;
            var p = cart[j];
            html += '<tr id="tr-' + p.link + '" title="' + p.link + '"><td>';
            if (p.icon) {
                html += CPage.createImgHtml(CBrowser.getB64DataFromLink(p.icon));
            }
            html += '</td><td>';
            html += p.name.substr(0, 32);
            html += '</td><td>';
            if (p.seller) {
                if (p.seller.domain)
                {
                    if (p.seller.domain.alias)
                        html += p.seller.domain.alias;
                    else
                        html += p.seller.domain.domain;
                }
                else if (p.seller.id)
                    html += showID(p.seller.id);
            }
            html += '</td><td>';
            html += ("φ" + p.price);
            html += '</td><td>';
            html += ("φ" + p.shipmentfee);
            html += '</td><td>';
            html += '<input name="quantity" type="text" style="width:40px" title="' + p.link + '" value="1" onkeyup="this.value=this.value.replace(/[^0-9]+/g,\'\')" onafterpaste="this.value=this.value.replace(/[^0-9]+/g,\'\')"/></td>';
            html += '<td><div class="subtotal" style="display:inline">' + "φ" + (p.price + p.shipmentfee) + '</div></td>';
            html += '<td><button class="idbtn cancel" title="' + p.link + '">x</button>';
            html += '</td></tr>';
        }

        html += "<tr><td>" + TR('Total') + "</td><td></td><td></td><td></td><td></td><td></td><td id='cart-total'></td><td></td></tr>";
        if (!hasContent)
            html = "<tr><td>" + TR('no products in cart') + ".</td></tr>";

        $('#cart-list').html(html);
        $("#cart-list tr").find("input[name='quantity']").unbind().bind("keyup change blur", function () {
            if (isNaN($(this).val()) || $(this).val() <= 0)
                $(this).val("");
            console.log($(this).parent());
            console.log($(this).parent().parent().find(".subtotal"));
            $(this).parent().next().html($(this).val() * cart[$(this).attr("title")].price + cart[$(this).attr("title")].shipmentfee);
            i.calCartTotal();
        });
        $("#cart-list").find(".cancel").unbind().click(function () {
            //console.log($(this));

            for (var j in cart) {
                //console.log(cart[j].link);
                //console.log($(this).parent().parent().attr("id"));
                if ($(this).parent().parent().attr("id") == "tr-" + cart[j].link) {
                    delete cart[j];
                    // console.log(j);
                    FAI_API.writeFile("shop", "", "cart", JSON.stringify(cart));
                    Shop.refreshCart();
                    break;
                }
            }
            //$(this).parent().parent().remove();
        });
        this.calCartTotal();
    }
    this.calCartTotal = function () {
        var total = 0;
        $("#cart-list").find("tr").each(function () {
            if ($(this).attr("title")) {
                var p = cart[$(this).attr("title")];
                var q = $(this).find("input").val();
                var subtotal = p.price * q + p.shipmentfee;
                total += subtotal;
            }
        });
        $("#cart-total").html("φ" + total);
    };
    this.doBuyProducts = function () {
        var l = [];
        var fValid = true;
        $("#cart-list tr").each(function () {
            var q = {};
            var p = cart[$(this).attr("title")];
            if (p) {
                q.type = "CC_PAYMENT_TYPE_PRODUCT";
                q.productid = p.id;
                q.paytolink = p.link;
                q.price = p.price;
                q.quantity = parseInt($(this).find("input").val());
                if (isNaN(q.quantity) || q.quantity <= 0) {
                    CPage.showNotice(TR('Quantity less than 1'));
                    fValid = false;
                    return;
                }
                console.log(q);
                var found = false;
                for (var j in l)
                    if (l[j].recipient == p.recipient) {
                        l[j].paymentitems.push(q);
                        found = true;
                        if (l[j].type == "CC_PAYMENT_TYPE_SHOPPING") {
                            for (var k in l[j].paymentitems)
                                if (l[j].paymentitems[k].price < p.shipmentfee)
                                    l[j].paymentitems[k].price = p.shipmentfee;
                        }
                        // break;
                    }
                if (!found) {
                    var j = {};
                    j.recipient = p.recipient;
                    j.type = "CC_PAYMENT_TYPE_SHOPPING";
                    j.paymentitems = [];
                    var ship = {type: "CC_PAYMENT_TYPE_SHIPMENTFEE", paytolink: p.link, price: p.shipmentfee, quantity: 1};
                    j.paymentitems.push(ship);
                    j.paymentitems.push(q);
                    l.push(j);
                } else {

                }
            }
        });
        if (!fValid)
            return;
        var feerate = FAI_API.getFeeRate(0.15);
        FAI_API.buyProducts(accountID, l, feerate, function (a) {
            CPage.showNotice(TR('Purchase successful'));
            cart = {};
            i.refreshCart();
            FAI_API.writeFile("shop", "", "cart", JSON.stringify(cart));
        }, function (r) {
            CPage.showNotice(r ? TR(r) : TR('Purchase failed'));
        });
    };

    function bindReady() {
        if (haveBoundReady) {
            return;
        }
        haveBoundReady = true;
        $("#btn-add-new").unbind().click(function () {
            var d = {};
            d.id = $("#add-new").find("input[name='product-id']").val().toString();
            var productname = $("#add-new").find("input[name='product-name']").val();
            if (!productname) {
                CPage.showNotice(TR('Empty product name'));
                return
            }
            d.name = productname;
            var price = $("#add-new").find("input[name='price']").val();
            if (isNaN(price)) {
                CPage.showNotice(TR('invalid price'));
                return
            }
            d.price = price;
            var shipmentfee = $("#add-new").find("input[name='shipment-fee']").val();
            if (shipmentfee) {
                if (isNaN(shipmentfee)) {
                    CPage.showNotice(TR('invalid shipment fee'));
                    return
                }
                d.shipmentfee = shipmentfee;
            }
            var recipient = $("#add-new").find("input[name='recipient']").val();
            if (recipient)
                d.recipient = recipient;
            var expiretime = $("#add-new").find("input[name='expire-time']").val();
            if (expiretime) {
                if (isNaN(expiretime) || expiretime > 3650 || expiretime < 0) {
                    CPage.showNotice(TR('invalid expire time'));
                    return
                }
                expiretime = expiretime * 3600 * 24 + Math.ceil((new Date()) / 1000);
                d.expiretime = expiretime;
            } else
                expiretime = 0;
            var icon = $("#add-new").find("input[name='icon']").val();
            if (icon)
                d.icon = icon;
            var intro = $("#add-new").find("textarea[name='intro']").val();
            if (intro)
                d.intro = intro;
            var tags = $("#add-new").find("textarea[name='tags']").val();
            if (tags)
                d.tags = tags.split(",");
            var lockvalue = $("#add-new").find("input[name='lockvalue']").val();
            var locktime = $("#add-new").find("input[name='locktime']").val();
            if (lockvalue && locktime)
            {
                d.lockvalue = lockvalue;
                d.locktime = locktime * 3600 * 24 + Math.ceil((new Date()) / 1000);
            }
            var feerate = FAI_API.getFeeRate(0.15);
            FAI_API.publishProduct(accountID, d, lockvalue, feerate, function (data) {
                CPage.showNotice(TR('Product published!'));
            }, function (e) {
                CPage.showNotice(TR("Publish product failed: ") + TR(e));
            });
        });
        $("#modal-product-details").find(".cancel").unbind().click(function () {
            $("#modal-product-details").modal("hide");
        });
        $("#modal-product-details").find("#btn-addcart").unbind().click(function () {
            $("#modal-product-details").modal("hide");
            for (var j in cart) {
                if (j == currentProduct.link)
                    return;
            }
            cart[currentProduct.link] = currentProduct;
            FAI_API.writeFile("shop", "", "cart", JSON.stringify(cart));
        });

        $("#btn-cart-buy").unbind().click(function () {
            i.doBuyProducts();
        });
        $("#btn-product-search").unbind().click(function () {
            var tags = $('#view-products').find('input[name="search"]').val();
            if (tags.length > 0)
                tags = tags.split(",");
            i.showViewProduct(tags);
        });
        $("#btn-test").click(function () {
            CPage.showNotice("test");
        });


        i.showViewProduct();
        i.shopOverView();
    }
    function initAccount() {
        accountID = FAI_API.getAccountID();
    }
    this.getPageName = function () {
        if (CUtil.getGet("buy"))
            return "buy";
        return "shop";
    };
    $(document).ready(function () {
        $("#tpls").load("templates.html", function () {
            CUtil.initGParam();
            CPage.prepareHeader(true);
            prepareStdTpl();
            doTranslate();
            $("input,button,select").attr("autocomplete", "off");
            bindInitial();
            initAccount();
            registerNotifications();
            bindReady();
            CPage.prepareNotice("shop");
            CPage.updateBalance();
            CPage.updateCblc();
            CPage.registerNotifications();
            cart = $.parseJSON(FAI_API.readFile("shop", "", "cart"));
            if (!cart)
                cart = {};
            var page = Shop.getPageName();
            console.log(page);
            if (page === "buy") {
                var link = CUtil.getGet("buy");
                console.log(link);
                // get prod by link
                var prod = FAI_API.getProductByLink(link);
                console.log(prod);
                cart[link] = prod;
                FAI_API.writeFile("shop", "", "cart", JSON.stringify(cart));
                $("#shopping-cart-btn").click();
            }
        });
    });
}