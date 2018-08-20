#include <iostream>
#include <string>

#include "test_tools.h"
#include "jinja2cpp/template.h"
#include "jinja2cpp/filesystem_handler.h"
#include "jinja2cpp/template_env.h"

class ExtendsTest : public testing::Test
{
public:
    void SetUp() override
    {
        m_templateFs = std::make_shared<jinja2::MemoryFileSystem>();
        m_env.AddFilesystemHandler(std::string(), m_templateFs);
    }

protected:
    std::shared_ptr<jinja2::MemoryFileSystem> m_templateFs;
    jinja2::TemplateEnv m_env;
};

TEST_F(ExtendsTest, BasicExtends)
{
    m_templateFs->AddFile("base.j2tpl", "Hello World!");
    m_templateFs->AddFile("derived.j2tpl", R"({% extends "base.j2tpl" %})");

    auto baseTpl = m_env.LoadTemplate("base.j2tpl").value();
    auto tpl = m_env.LoadTemplate("derived.j2tpl").value();

    std::string baseResult = baseTpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << baseResult << std::endl;
    std::string expectedResult = "Hello World!";
    EXPECT_STREQ(expectedResult.c_str(), baseResult.c_str());
    std::string result = tpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << result << std::endl;
    EXPECT_STREQ(baseResult.c_str(), result.c_str());
}

TEST_F(ExtendsTest, SimpleBlockExtends)
{
    m_templateFs->AddFile("base.j2tpl", "Hello World! ->{% block b1 %}{% endblock %}<-");
    m_templateFs->AddFile("derived.j2tpl", R"({% extends "base.j2tpl" %}{%block b1%}Extended block!{%endblock%})");

    auto baseTpl = m_env.LoadTemplate("base.j2tpl").value();
    auto tpl = m_env.LoadTemplate("derived.j2tpl").value();

    std::string baseResult = baseTpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << baseResult << std::endl;
    std::string expectedResult = "Hello World! -><-";
    EXPECT_STREQ(expectedResult.c_str(), baseResult.c_str());
    std::string result = tpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << result << std::endl;
    expectedResult = "Hello World! ->Extended block!<-";
    EXPECT_STREQ(expectedResult.c_str(), result.c_str());
}

TEST_F(ExtendsTest, TwoLevelBlockExtends)
{
    m_templateFs->AddFile("base.j2tpl", "Hello World! ->{% block b1 %}{% endblock %}<-");
    m_templateFs->AddFile("derived.j2tpl", R"({% extends "base.j2tpl" %}{%block b1%}Extended block!{%block innerB1%}=>innerB1 content<={%endblock%}{%endblock%})");
    m_templateFs->AddFile("derived2.j2tpl", R"({% extends "derived.j2tpl" %}{%block innerB1%}derived2 block{{super()}}{%endblock%})");

    auto baseTpl = m_env.LoadTemplate("base.j2tpl").value();
    auto tpl = m_env.LoadTemplate("derived.j2tpl").value();
    auto tpl2 = m_env.LoadTemplate("derived2.j2tpl").value();

    std::string baseResult = baseTpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << baseResult << std::endl;
    std::string expectedResult = "Hello World! -><-";
    EXPECT_STREQ(expectedResult.c_str(), baseResult.c_str());
    std::string result = tpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << result << std::endl;
    expectedResult = "Hello World! ->Extended block!<-";
    EXPECT_STREQ(expectedResult.c_str(), result.c_str());
    std::string result2 = tpl2.RenderAsString(jinja2::ValuesMap{});
    std::cout << result2 << std::endl;
    expectedResult = "Hello World! ->Extended block!derived2 block=>innerB1 content<=<-";
    EXPECT_STREQ(expectedResult.c_str(), result2.c_str());
}

TEST_F(ExtendsTest, DoubleBlocksExtends)
{
    m_templateFs->AddFile("base.j2tpl", "Hello World! ->{% block b1 %}{% endblock %}<- ->{% block b2 %}{% endblock b2%}<-");
    m_templateFs->AddFile("derived.j2tpl", R"({% extends "base.j2tpl" %}{%block b1%}Extended block b1!{%endblock%}Some Stuff{%block b2%}Extended block b2!{%endblock%})");

    auto baseTpl = m_env.LoadTemplate("base.j2tpl").value();
    auto tpl = m_env.LoadTemplate("derived.j2tpl").value();

    std::string baseResult = baseTpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << baseResult << std::endl;
    std::string expectedResult = "Hello World! -><- -><-";
    EXPECT_STREQ(expectedResult.c_str(), baseResult.c_str());
    std::string result = tpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << result << std::endl;
    expectedResult = "Hello World! ->Extended block b1!<- ->Extended block b2!<-";
    EXPECT_STREQ(expectedResult.c_str(), result.c_str());
}

TEST_F(ExtendsTest, SuperBlocksExtends)
{
    m_templateFs->AddFile("base.j2tpl", "Hello World! ->{% block b1 %}=>block b1<={% endblock %}<- ->{% block b2 %}{% endblock b2%}<-");
    m_templateFs->AddFile("derived.j2tpl", R"({% extends "base.j2tpl" %}{%block b1%}Extended block b1!{{super()}}{%endblock%}Some Stuff{%block b2%}Extended block b2!{%endblock%})");

    auto baseTpl = m_env.LoadTemplate("base.j2tpl").value();
    auto tpl = m_env.LoadTemplate("derived.j2tpl").value();

    std::string baseResult = baseTpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << baseResult << std::endl;
    std::string expectedResult = "Hello World! -><- -><-";
    EXPECT_STREQ(expectedResult.c_str(), baseResult.c_str());
    std::string result = tpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << result << std::endl;
    expectedResult = "Hello World! ->Extended block b1!=>block b1<=<- ->Extended block b2!<-";
    EXPECT_STREQ(expectedResult.c_str(), result.c_str());
}

TEST_F(ExtendsTest, SuperAndSelfBlocksExtends)
{
    m_templateFs->AddFile("base.j2tpl", R"(Hello World!{%set testVal='first entry' %}
->{% block b1 scoped %}=>block b1 - {{testVal}}<={% endblock %}<-
-->{{self.b1()}}<---->{{self.b2()}}<--{%set testVal='second entry' %}
->{% block b2 %}{% endblock b2%}<-
-->{{self.b1()}}<---->{{self.b2()}}<--
)");
    m_templateFs->AddFile("derived.j2tpl", R"({% extends "base.j2tpl" %}{%block b1%}Extended block b1!{{super()}}{%endblock%}Some Stuff{%block b2%}Extended block b2!{%endblock%})");

    auto baseTpl = m_env.LoadTemplate("base.j2tpl").value();
    auto tpl = m_env.LoadTemplate("derived.j2tpl").value();

    std::string baseResult = baseTpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << baseResult << std::endl;
    std::string expectedResult = R"(Hello World!-><-
--><----><---><-
--><----><--
)";
    EXPECT_STREQ(expectedResult.c_str(), baseResult.c_str());
    std::string result = tpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << result << std::endl;
    expectedResult = R"(Hello World!->Extended block b1!=>block b1 - first entry<=<-
-->Extended block b1!=>block b1 - first entry<=<----><--->Extended block b2!<-
-->Extended block b1!=>block b1 - second entry<=<---->Extended block b2!<--
)";
    EXPECT_STREQ(expectedResult.c_str(), result.c_str());
}

TEST_F(ExtendsTest, InnerBlocksExtends)
{
    m_templateFs->AddFile("base.j2tpl", R"(Hello World!{%set testVal='first entry' %}
->{% block b1 scoped %}=>block b1 - {{testVal}}<={%block innerB1 scoped%}{%endblock%}{% endblock %}<-
-->{{self.b1()}}<---->{{self.b2()}}<--{%set testVal='second entry' %}
->{% block b2 %}{{self.innerB1()}}{% endblock b2%}<-
-->{{self.b1()}}<---->{{self.b2()}}<--
)");
    m_templateFs->AddFile("derived.j2tpl", R"({% extends "base.j2tpl" %}
{%block b1%}Extended block b1!{{super()}}{%endblock%}
{%block b2%}Extended block b2!{{super()}}{%endblock%}
{%block innerB1%}###Extended innerB1 block {{testVal}}!###{%endblock%}
)");

    auto baseTpl = m_env.LoadTemplate("base.j2tpl").value();
    auto tpl = m_env.LoadTemplate("derived.j2tpl").value();

    std::string baseResult = baseTpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << baseResult << std::endl;
    std::string expectedResult = R"(Hello World!-><-
--><----><---><-
--><----><--
)";
    EXPECT_STREQ(expectedResult.c_str(), baseResult.c_str());
    std::string result = tpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << result << std::endl;
    expectedResult = R"(Hello World!->Extended block b1!=>block b1 - first entry<=###Extended innerB1 block first entry!###<-
-->Extended block b1!=>block b1 - first entry<=###Extended innerB1 block first entry!###<----><--->Extended block b2!<-
-->Extended block b1!=>block b1 - second entry<=###Extended innerB1 block second entry!###<---->Extended block b2!<--
)";
    EXPECT_STREQ(expectedResult.c_str(), result.c_str());
}

TEST_F(ExtendsTest, ScopedBlocksExtends)
{
    m_templateFs->AddFile("base.j2tpl", R"(Hello World!
->{% for i in range(10) %}{% block b1 %}{% endblock %}{%endfor%}<-
->{% for i in range(10) %}{% block b2 scoped %}{% endblock b2%}{%endfor%}<-)");
    m_templateFs->AddFile("derived.j2tpl", R"({% extends "base.j2tpl" %}{%block b1%}{{i}}{%endblock%}Some Stuff{%block b2%}{{i}}{%endblock%})");

    auto baseTpl = m_env.LoadTemplate("base.j2tpl").value();
    auto tpl = m_env.LoadTemplate("derived.j2tpl").value();

    std::string baseResult = baseTpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << baseResult << std::endl;
    std::string expectedResult = "Hello World!\n-><-\n-><-";
    EXPECT_STREQ(expectedResult.c_str(), baseResult.c_str());
    std::string result = tpl.RenderAsString(jinja2::ValuesMap{});
    std::cout << result << std::endl;
    expectedResult = R"(Hello World!
-><-
->0123456789<-)";
    EXPECT_STREQ(expectedResult.c_str(), result.c_str());
}