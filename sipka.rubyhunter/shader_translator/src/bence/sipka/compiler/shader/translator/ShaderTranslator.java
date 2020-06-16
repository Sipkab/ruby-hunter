/*
 * Copyright (C) 2020 Bence Sipka
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package bence.sipka.compiler.shader.translator;

import java.io.IOException;
import java.io.PrintStream;
import java.util.NavigableMap;

import bence.sipka.compiler.shader.ShaderCollection;
import bence.sipka.compiler.shader.statement.AssignmentStatement;
import bence.sipka.compiler.shader.statement.BlockStatement;
import bence.sipka.compiler.shader.statement.VarDeclarationStatement;
import bence.sipka.compiler.shader.statement.control.IfControlStatement;
import bence.sipka.compiler.shader.statement.control.WhileControlStatement;
import bence.sipka.compiler.shader.statement.expression.BoolLiteral;
import bence.sipka.compiler.shader.statement.expression.FloatLiteral;
import bence.sipka.compiler.shader.statement.expression.FunctionCallStatement;
import bence.sipka.compiler.shader.statement.expression.IntegerLiteral;
import bence.sipka.compiler.shader.statement.expression.ParenthesesStatement;
import bence.sipka.compiler.shader.statement.expression.VariableExpression;
import saker.build.file.SakerDirectory;
import saker.build.file.path.SakerPath;
import saker.build.task.TaskContext;

public abstract class ShaderTranslator {

	private String uniqueName;

	public ShaderTranslator(String uniquename) {
		this.uniqueName = uniquename;
	}

	public abstract void translate(TaskContext taskcontext, SakerDirectory sourcesdir, SakerDirectory shadersdir,
			ShaderCollection shaders, NavigableMap<String,SakerPath> assets) throws IOException;

	public abstract void putVariableDeclaration(VarDeclarationStatement vardecl, PrintStream os);

	public abstract void putVariable(VariableExpression variable, PrintStream os);

	public abstract void putParentheses(ParenthesesStatement parentheses, PrintStream os);

	public abstract void putOperatorExpression(FunctionCallStatement operator, PrintStream os);

	public abstract void putBlockStatement(BlockStatement block, PrintStream os);

	public abstract void putFunctionCall(FunctionCallStatement functioncall, PrintStream os);

	public abstract void putAssignment(AssignmentStatement assignment, PrintStream os);

	public abstract void putIfControl(IfControlStatement ifControl, PrintStream os);

	public abstract void putWhileControl(WhileControlStatement whileControl, PrintStream os);

	public abstract void putFloatLiteral(FloatLiteral floatLiteral, PrintStream os);

	public abstract void putIntegerLiteral(IntegerLiteral integerLiteral, PrintStream os);

	public abstract void putBoolLiteral(BoolLiteral boolLiteral, PrintStream os);

	public final String getUniqueName() {
		return uniqueName;
	}
}
