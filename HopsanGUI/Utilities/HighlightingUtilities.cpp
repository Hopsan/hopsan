/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   HighlightingUtilities.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-08-23
//!
//! @brief Contains highlighter classes for various languages
//!
//$Id$

#include "common.h"
#include "global.h"
#include "HcomHandler.h"
#include "HighlightingUtilities.h"
#include "Widgets/HcomWidget.h"


XmlHighlighter::XmlHighlighter(QTextDocument* parent)
: QSyntaxHighlighter(parent)
{
    mSyntaxChar.setForeground(Qt::blue);
    mElementName.setForeground(Qt::darkRed);
    mComment.setForeground(Qt::darkGreen);
    mAttributeName.setForeground(Qt::red);
    mAttributeValue.setForeground(Qt::blue);
    mError.setForeground(Qt::darkMagenta);
    mOther.setForeground(Qt::black);
}


void XmlHighlighter::setHighlightColor(HighlightType type, QColor color, bool foreground)
{
	QTextCharFormat format;
	if (foreground)
		format.setForeground(color);
	else
		format.setBackground(color);
	setHighlightFormat(type, format);
}

void XmlHighlighter::setHighlightFormat(HighlightType type, QTextCharFormat format)
{
	switch (type)
	{
		case SyntaxChar:
            mSyntaxChar = format;
			break;
		case ElementName:
            mElementName = format;
			break;
		case Comment:
            mComment = format;
			break;
		case AttributeName:
            mAttributeName = format;
			break;
		case AttributeValue:
            mAttributeValue = format;
			break;
		case Error:
            mError = format;
			break;
		case Other:
            mOther = format;
			break;
	}
	rehighlight();
}

void XmlHighlighter::highlightBlock(const QString& text)
{
	int i = 0;
	int pos = 0;
	int brackets = 0;
	
	state = (previousBlockState() == InElement ? ExpectAttributeOrEndOfElement : NoState);

	if (previousBlockState() == InComment)
	{
		// search for the end of the comment
        QRegExp expression("[^-]*-([^-][^-]*-)*->");
		pos = expression.indexIn(text, i);

		if (pos >= 0)
		{
			// end comment found
			const int iLength = expression.matchedLength();
            setFormat(0, iLength - 3, mComment);
            setFormat(iLength - 3, 3, mSyntaxChar);
			i += iLength; // skip comment
		}
		else
		{
			// in comment
            setFormat(0, text.length(), mComment);
			setCurrentBlockState(InComment);
			return;
		}
	}

	for (; i < text.length(); i++)
	{
        switch (text.at(i).toLatin1())
		{
		case '<':
			brackets++;
			if (brackets == 1)
			{
                setFormat(i, 1, mSyntaxChar);
				state = ExpectElementNameOrSlash;
			}
			else
			{
				// wrong bracket nesting
                setFormat(i, 1, mError);
			}
			break;

		case '>':
			brackets--;
			if (brackets == 0)
			{
                setFormat(i, 1, mSyntaxChar);
			}
			else
			{
				// wrong bracket nesting
                setFormat( i, 1, mError);
			}
			state = NoState;
			break;

		case '/':
			if (state == ExpectElementNameOrSlash)
			{
				state = ExpectElementName;
                setFormat(i, 1, mSyntaxChar);
			}
			else
			{
				if (state == ExpectAttributeOrEndOfElement)
				{
                    setFormat(i, 1, mSyntaxChar);
				}
				else
				{
					processDefaultText(i, text);
				}
			}
			break;

		case '=':
			if (state == ExpectEqual)
			{
				state = ExpectAttributeValue;
                setFormat(i, 1, mOther);
			}
			else
			{
				processDefaultText(i, text);  
			}
			break;

		case '\'':
		case '\"':
			if (state == ExpectAttributeValue)
			{
				// search attribute value
                QRegExp expression("\"[^<\"]*\"|'[^<']*'");
				pos = expression.indexIn(text, i);

				if (pos == i) // attribute value found ?
				{
					const int iLength = expression.matchedLength();

                    setFormat(i, 1, mOther);
                    setFormat(i + 1, iLength - 2, mAttributeValue);
                    setFormat(i + iLength - 1, 1, mOther);

					i += iLength - 1; // skip attribute value
					state = ExpectAttributeOrEndOfElement;
				}
				else
				{
					processDefaultText(i, text);
				}
			}
			else
			{
				processDefaultText(i, text);
			}
			break;

		case '!':
			if (state == ExpectElementNameOrSlash)
			{
				// search comment
                QRegExp expression("<!--[^-]*-([^-][^-]*-)*->");
				pos = expression.indexIn(text, i - 1);

				if (pos == i - 1) // comment found ?
				{
					const int iLength = expression.matchedLength();

                    setFormat(pos, 4, mSyntaxChar);
                    setFormat(pos + 4, iLength - 7, mComment);
                    setFormat(iLength - 3, 3, mSyntaxChar);
					i += iLength - 2; // skip comment
					state = NoState;
					brackets--;
				}
				else
				{
					// Try find multiline comment
                    QRegExp expression("<!--"); // search comment start
					pos = expression.indexIn(text, i - 1);

					//if (pos == i - 1) // comment found ?
					if (pos >= i - 1)
					{
                        setFormat(i, 3, mSyntaxChar);
                        setFormat(i + 3, text.length() - i - 3, mComment);
						setCurrentBlockState(InComment);
						return;
					}
					else
					{
						processDefaultText(i, text);
					}
				}
			}
			else
			{
				processDefaultText(i, text);
			}

			break;  

		default:
			const int iLength = processDefaultText(i, text);
			if (iLength > 0)
				i += iLength - 1;
			break;
		}
	}

	if (state == ExpectAttributeOrEndOfElement)
	{
		setCurrentBlockState(InElement);
	}
}

int XmlHighlighter::processDefaultText(int i, const QString& text)
{
	// length of matched text
	int iLength = 0;

	switch(state)
	{
	case ExpectElementNameOrSlash:
	case ExpectElementName:
		{
			// search element name
            QRegExp expression("([A-Za-z_:]|[^\\x00-\\x7F])([A-Za-z0-9_:.-]|[^\\x00-\\x7F])*");
			const int pos = expression.indexIn(text, i);

			if (pos == i) // found ?
			{
				iLength = expression.matchedLength();

                setFormat(pos, iLength, mElementName);
				state = ExpectAttributeOrEndOfElement;
			}
			else
			{
                setFormat(i, 1, mOther);
			}
		}  
		break;

	case ExpectAttributeOrEndOfElement:
		{
			// search attribute name
            QRegExp expression("([A-Za-z_:]|[^\\x00-\\x7F])([A-Za-z0-9_:.-]|[^\\x00-\\x7F])*");
			const int pos = expression.indexIn(text, i);

			if (pos == i) // found ?
			{
				iLength = expression.matchedLength();

                setFormat(pos, iLength, mAttributeName);
				state = ExpectEqual;
			}
			else
			{
                setFormat(i, 1, mOther);
			}
		}
		break;

	default:
        setFormat(i, 1, mOther);
		break;
	}
	return iLength;
}  




CppHighlighter::CppHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    //mFunctionFormat.setFontWeight(75);
    mFunctionFormat.setForeground(Qt::black);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = mFunctionFormat;
    mHighlightingRules.append(rule);

    mKeywordFormat.setForeground(Qt::darkYellow);
    mKeywordFormat.setFontWeight(QFont::Normal);
    QStringList keywordPatterns;
    keywordPatterns << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\bshort\\b" << "\\bsigned\\b" << "\\bstatic\\b"
                    << "\\bstruct\\b" << "\\btemplate\\b" << "\\btypedef\\b"
                    << "\\btypename\\b" << "\\bunion\\b" << "\\bunsigned\\b"
                    << "\\bvirtual\\b" << "\\bvoid\\b" << "\\bvolatile\\b"
                    << "\\busing\\b" << "\\bbool\\b" << "\\bif\\b"
                    << "\\belse\\b" << "\\bfor\\b" << "\\bwhile\\b"
                    << "\\bswitch\\b" << "\\bcase\\b" << "\\bdefault\\b"
                    << "\\bbreak\\b" << "\\breturn\\b" << "\\bif\\()"
                    << "\\bconstexpr\\b";
    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = mKeywordFormat;
        mHighlightingRules.append(rule);
    }

    mHopsanKeywordFormat.setForeground(Qt::darkMagenta);
    mHopsanKeywordFormat.setFontWeight(QFont::Normal);
    QStringList hopsanKeywordPatterns;
    hopsanKeywordPatterns << "\\bPort\\b" << "\\bFirstOrderTransferFunctionVariable\\b" << "\\bSecondOrderTransferFunctionVariable\\b"
                          << "\\bFirstOrderTransferFunction\\b" << "\\bSecondOrderTransferFunction\\b"
                          << "\\bFirstOrderFiler\\b" << "\\bSecondOrderFilter\\b" << "\\bDelay\\b"
                          << "\\bIntegrator\\b" << "\\bIntegratorLimited\\b" << "\\bTurbulentFlowFunction\\b"
                          << "\\bValveHysteresis\\b" << "\\bDoubleIntegratorWithDamping\\b" << "\\bDoubleIntegratorWithDampingAndCoulumbFriction\\b"
                          << "\\bCSVParser\\b" << "\\bCSVParserNG\\b" << "\\bPLOParser\\b"
                          << "\\bWhiteGaussianNoise\\b" << "\\bEquationSystemSolver\\b" << "\\bNumericalIntegrationSolver\\b"
                          << "\\bLookupTable1D\\b" << "\\bLookupTable2D\\b" << "\\bLookupTable3D\\b";

    foreach (const QString &pattern, hopsanKeywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = mHopsanKeywordFormat;
        mHighlightingRules.append(rule);
    }

    mPreProcessorFormat.setForeground(Qt::darkBlue);
    mPreProcessorFormat.setFontWeight(QFont::Normal);
    keywordPatterns.clear();
    keywordPatterns << "^#?\\binclude\\b" << "^#?\\bifdef\\b" << "^#?\\bifndef\\b"
                    << "^#?\\belseif\\b" << "^#?\\belse\\b" << "^#?\\bendif\\b";
    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = mPreProcessorFormat;
        mHighlightingRules.append(rule);
    }

    //Make Qt classes purple, no real point in component libraries
//    mClassFormat.setFontWeight(QFont::Bold);
//    mClassFormat.setForeground(Qt::darkMagenta);
//    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
//    rule.format = mClassFormat;
//    mHighlightingRules.append(rule);

    mSingleLineCommentFormat.setForeground(Qt::gray);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = mSingleLineCommentFormat;
    mHighlightingRules.append(rule);

    mMultiLineCommentFormat.setForeground(Qt::gray);

    mQuotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("<.*>");
    rule.format = mQuotationFormat;
    mHighlightingRules.append(rule);

    mTagFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = mQuotationFormat;
    mHighlightingRules.append(rule);

    mCommentStartExpression = QRegExp("/\\*");
    mCommentEndExpression = QRegExp("\\*/");
}

void CppHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, mHighlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = mCommentStartExpression.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = mCommentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                    + mCommentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, mMultiLineCommentFormat);
        startIndex = mCommentStartExpression.indexIn(text, startIndex + commentLength);
    }
}





ModelicaHighlighter::ModelicaHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    //Functions
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns  << "\\bVariableLimits\\b" << "\\bVariable2Limits\\b";       //These are special and will not be treated as normal functions later on
    QStringList functions = getSupportedFunctionsList();
    for(int i=0; i<functions.size(); ++i)
    {
        keywordPatterns << "\\b"+functions[i]+"\\b";
    }

    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    //Model structure keywords
    keywordFormat.setForeground(Qt::darkRed);
    keywordFormat.setFontWeight(QFont::Bold);
    keywordPatterns.clear();
    keywordPatterns  << "\\bmodel\\b" << "\\bequation\\b" << "\\balgorithm\\b" << "\\bend\\b" << "\\bannotation\\b" << "\\bconnector\\b";

    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    //Parameter and type keywords
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    keywordPatterns.clear();
    keywordPatterns  << "\\bparameter\\b" << "\\bReal\\b";

    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    //Comments
    keywordFormat.setForeground(Qt::darkGreen);
    keywordFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = keywordFormat;
    highlightingRules.append(rule);

    //Quotations
    keywordFormat.setForeground(Qt::darkGreen);
    keywordFormat.setFontWeight(QFont::Normal);
    rule.pattern = QRegExp("\".*\"");
    rule.format = keywordFormat;
    highlightingRules.append(rule);
}

//! @brief Returns a list with supported functions for equation-based model generation
//! @todo Duplicated with HopsanGenerator
QStringList ModelicaHighlighter::getSupportedFunctionsList()
{
    return QStringList() << "div" << "rem" << "mod" << "tan" << "cos" << "sin" << "atan" << "acos" << "asin" << "atan2" << "sinh" << "cosh" << "tanh" << "log" << "exp" << "sqrt" << "sign" << "abs" << "der" << "onPositive" << "onNegative" << "signedSquareL" << "limit" << "integer" << "floor" << "ceil" << "pow" << "nonZero";
}

void ModelicaHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
}




PythonHighlighter::PythonHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkYellow);
    keywordFormat.setFontWeight(QFont::Normal);
    QStringList keywordPatterns;
    keywordPatterns << "\\band\\b" << "\\bas\\b" << "\\bassert\\b" << "\\bbreak\\b" << "\\bclass\\b"
        << "\\bcontinue\\b" << "\\bdef\\b" << "\\bdel\\b" << "\\belif\\b" << "\\belse\\b"
        << "\\bexcept\\b" << "\\bexec\\b" << "\\bfinally\\b" << "\\bfor\\b" << "\\bfrom\\b"
        << "\\bglobal\\b" << "\\bif\\b" << "\\bimport\\b" << "\\bin\\b" << "\\bis\\b"
        << "\\blambda\\b" << "\\bnot\\b" << "\\bor\\b" << "\\bpass\\b" << "\\bprint\\b" << "\\braise\\b"
        << "\\breturn\\b" << "\\btry\\b" << "\\bwhile\\b" << "\\bwith\\b" << "\\byield\\b" << "\\bTrue\\b"
        << "\\bFalse\\b" << "\\bNone\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }



    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::red);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("<.*>");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    tagFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);
}


void PythonHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
}


HcomHighlighter::HcomHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    commandFormat.setForeground(Qt::darkGreen);
    commandFormat.setFontWeight(QFont::Bold);
    QStringList commandPatterns = gpTerminalWidget->mpHandler->getCommands();
    commandPatterns << "define" << "enddefine" << "while" << "if" << "foreach" << "repeat" << "goto" << "end";
    for(int i=0; i<commandPatterns.size(); ++i)
    {
        commandPatterns[i].prepend("\\b");
        commandPatterns[i].append("\\b");
    }
    foreach (const QString &pattern, commandPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = commandFormat;
        highlightingRules.append(rule);
    }

    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::red);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("<.*>");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    tagFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontWeight(QFont::Bold);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);
}


void HcomHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
}



HighlighterTypeEnum highlighterForExtension(QString extension)
{
    HighlighterTypeEnum highlighter = HighlighterTypeEnum::PlainText;
    if(extension == "hcom") {
        highlighter = HighlighterTypeEnum::Hcom;
    }
    else if(extension == "hpp" || extension == "h" || extension == "cpp" ||
            extension == "cc" || extension == "c") {
        highlighter = HighlighterTypeEnum::Cpp;
    }
    else if(extension == "xml" || extension == "hmf") {
        highlighter =  HighlighterTypeEnum::XML;
    }
    else if(extension == "mo") {
        highlighter = HighlighterTypeEnum::Modelica;
    }
    else if(extension == "py") {
        highlighter = HighlighterTypeEnum::Python;
    }
    return highlighter;
}
