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
package bence.sipka.compiler.source;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.Parameter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Stack;
import java.util.stream.Collectors;

import bence.sipka.utils.BundleContentAccess;
import bence.sipka.utils.BundleContentAccess.BundleResourceSupplier;
import saker.build.thirdparty.saker.util.ReflectUtils;
import sipka.syntax.parser.model.ParseFailedException;
import sipka.syntax.parser.model.rule.Language;
import sipka.syntax.parser.model.statement.Statement;
import sipka.syntax.parser.util.Pair;

public class SourceTemplateTranslator {
	private static final String FUNCTION_PREFIX = "@function ";

	public static final String[] CPP_KEYWORDS = { "catch", "try", "unsigned", "int", "char", "short", "long",
			"unsigned", "signed", "float", "double", "bool", "void", "public", "private", "protected", "class",
			"struct", "enum", "union", "const", "goto", "break", "continue", "auto", "constexpr", "static_cast",
			"dynamic_cast", "const_cast", "reinterpret_cast", "true", "false", "this", "static", "static_assert",
			"sizeof", "return", "for", "do", "while", "if", "else", "switch", "case", "default", "new", "delete",
			"decltype", "inline", "friend", "extern", "namespace", "operator", "volatile", "virtual", "using",
			"typename", "template", "throw", "nullptr", "typedef" };

	static {
		Arrays.sort(CPP_KEYWORDS);
	}
//
//	public InputStream getBuiltinFileResourceStream(String path) throws FileNotFoundException {
//		InputStream result = getClass().getClassLoader().getResourceAsStream(path);
//		if (result == null) {
//			throw new FileNotFoundException("Built in file resource not found: " + path);
//		}
//		return result;
//	}

	@SuppressWarnings("unused")
	private static class TranslationFailedException extends RuntimeException {
		private static final long serialVersionUID = 1413766147878664235L;

		public TranslationFailedException() {
			super();
		}

		public TranslationFailedException(String message, Throwable cause) {
			super(message, cause);
		}

		public TranslationFailedException(String message) {
			super(message);
		}

		public TranslationFailedException(Throwable cause) {
			super(cause);
		}

	}

	@SuppressWarnings("unused")
	public interface TranslationHandler {
		default public void replaceKey(String key, OutputStream out) throws IOException {
			throw new TranslationFailedException("Key not handled: " + key);
		}

		default public TranslationHandler includeFile(String path, OutputStream out) throws IOException {
			return this;
		}
	}

	private static final BundleResourceSupplier descriptor = BundleContentAccess.getBundleResourceSupplier("sources");
	private static final Language TEMPLATE_LANG;
	static {
		Language lang = null;
		try {
			lang = Language.fromInputStream(descriptor.getInputStream("cpptemplate.lang")).get("cpptemplate_lang");
		} catch (ParseFailedException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		TEMPLATE_LANG = lang;
	}

	private static boolean isConvertibleTo(Class<?> from, Class<?> to) {
		return to.isAssignableFrom(from) || //
				((to.isPrimitive() || from.isPrimitive()) && //
						(to.equals(Byte.class) && from.equals(byte.class) || //
								to.equals(Character.class) && from.equals(char.class) || //
								to.equals(Short.class) && from.equals(short.class) || //
								to.equals(Integer.class) && from.equals(int.class) || //
								to.equals(Long.class) && from.equals(long.class) || //
								to.equals(Float.class) && from.equals(float.class) || //
								to.equals(Double.class) && from.equals(double.class) || //
								to.equals(Boolean.class) && from.equals(boolean.class) || //

								from.equals(Byte.class) && to.equals(byte.class) || //
								from.equals(Character.class) && to.equals(char.class) || //
								from.equals(Short.class) && to.equals(short.class) || //
								from.equals(Integer.class) && to.equals(int.class) || //
								from.equals(Long.class) && to.equals(long.class) || //
								from.equals(Float.class) && to.equals(float.class) || //
								from.equals(Double.class) && to.equals(double.class) || //
								from.equals(Boolean.class) && to.equals(boolean.class)));
	}

	private static class VarPair {
		String name;
		Object value;

		public VarPair(String name, Object value) {
			this.name = name;
			this.value = value;
		}

		@Override
		public String toString() {
			return "[" + name + " : " + value + "]";
		}

	}

	public interface IncludeResolver {
		public InputStream get(String path) throws IOException;
	}

	private StringBuilder wsbuffer = new StringBuilder();
	private TranslationHandler handler;
	private IncludeResolver includeResolver;

	public SourceTemplateTranslator(TranslationHandler handler) {
		this.handler = handler;
	}

	public SourceTemplateTranslator(TranslationHandler handler, IncludeResolver includeResolver) {
		this.handler = handler;
		this.includeResolver = includeResolver;
	}

	public SourceTemplateTranslator(IncludeResolver includeResolver) {
		this.includeResolver = includeResolver;
	}

	private static Method searchMethod(String name, Class<?> clazz, Object... args) throws NoSuchMethodException {
		Method res = null;
		foreach:
		for (Method m : clazz.getMethods()) {
			Parameter[] params = m.getParameters();
			if (params.length != args.length) {
				continue;
			}
			if (!m.getName().equals(name)) {
				continue;
			}
			for (int i = 0; i < args.length; i++) {
				Class<?> paramtype = params[i].getType();
				if (paramtype.isPrimitive() && (args[i] == null)) {
					continue foreach;
				}
				if (!isConvertibleTo(args[i].getClass(), paramtype)) {
					continue foreach;
				}
			}
			res = m;
		}
		if (res == null) {
			throw new NoSuchMethodException(
					"No method: " + name + " in class: " + clazz + " for args: " + Arrays.toString(args));
		}
		if (!Modifier.isPublic(res.getModifiers()) || !Modifier.isPublic(res.getDeclaringClass().getModifiers())) {
			Class<?>[] paramtypes = res.getParameterTypes();
			for (Class<?> type : ReflectUtils.getAllInheritedTypes(clazz)) {
				if (!Modifier.isPublic(type.getModifiers())) {
					continue;
				}
				Method found;
				try {
					found = type.getMethod(name, paramtypes);
				} catch (NoSuchMethodException e) {
					continue;
				}
				if (Modifier.isPublic(found.getModifiers())) {
					//found a public method on a public type
					return found;
				}
			}
		}
		return res;
	}

	private static Constructor<?> searchConstructor(Class<?> clazz, Object... args) throws NoSuchMethodException {
		Constructor<?> res = null;
		foreach:
		for (Constructor<?> m : clazz.getConstructors()) {
			Parameter[] params = m.getParameters();
			if (params.length != args.length) {
				continue;
			}
			for (int i = 0; i < args.length; i++) {
				Class<?> paramtype = params[i].getType();
				if (paramtype.isPrimitive() && (args[i] == null)) {
					continue foreach;
				}
				if (!isConvertibleTo(args[i].getClass(), paramtype)) {
					continue foreach;
				}
			}
			res = m;
		}
		if (res == null) {
			throw new NoSuchMethodException(
					"No constructor in class: " + clazz + " for args: " + Arrays.toString(args));
		}
		return res;
	}

	private static Field searchField(Class<?> clazz, String name) throws NoSuchFieldException, SecurityException {
		do {
			try {
				return clazz.getDeclaredField(name);
			} catch (NoSuchFieldException e) {
				clazz = clazz.getSuperclass();
			}
		} while (clazz != null);
		throw new NoSuchFieldException(name);
	}

	private static Object findValue(String name, Stack<VarPair> varstack) {
		if ("null".equals(name)) {
			return null;
		}
		for (int i = varstack.size() - 1; i >= 0; i--) {
			VarPair pair = varstack.get(i);
			if (pair.name.equals(name)) {
				return pair.value;
			}
		}
		return null;
	}

	private static VarPair findPairValue(String name, Stack<VarPair> varstack) throws TranslationFailedException {
		if ("null".equals(name)) {
			return null;
		}
		for (int i = varstack.size() - 1; i >= 0; i--) {
			VarPair pair = varstack.get(i);
			if (pair.name.equals(name)) {
				return pair;
			}
		}
		throw new TranslationFailedException("Variable not found with name: " + name);
	}

	private static Object executeOperator(String operator, Object left, Object right)
			throws TranslationFailedException {
		Object result = left;
		switch (operator) {
			case "==": {
				if (result != null && right != null) {
					if (result instanceof Number && right instanceof Number) {
						Number leftn = (Number) result;
						Number rightn = (Number) right;
						if (leftn.getClass() == Double.class || leftn.getClass() == Float.class
								|| rightn.getClass() == Double.class || rightn.getClass() == Float.class) {
							return leftn.doubleValue() == rightn.doubleValue();
						}
						return leftn.longValue() == rightn.longValue();
					}
				}
				return result == right;
			}
			case "+": {
				if (left == null || (left.getClass() != String.class && right == null)) {
					throw new TranslationFailedException(
							"Unable to execute operator: " + left + " " + operator + " " + right);
				}
				if (left.getClass() == String.class) {
					return ((String) left) + right;
				}
				if (left.getClass() == Character.class)
					left = (int) left;
				if (right.getClass() == Character.class)
					right = (int) right;
				if (result instanceof Number && right instanceof Number) {
					Number leftn = (Number) result;
					Number rightn = (Number) right;
					if (leftn.getClass() == Double.class || rightn.getClass() == Double.class) {
						return leftn.doubleValue() + rightn.doubleValue();
					}
					if (leftn.getClass() == Float.class || rightn.getClass() == Float.class) {
						return leftn.floatValue() + rightn.floatValue();
					}
					if (leftn.getClass() == Long.class || rightn.getClass() == Long.class) {
						return leftn.longValue() + rightn.longValue();
					}
					if (leftn.getClass() == Integer.class || rightn.getClass() == Integer.class) {
						return leftn.intValue() + rightn.intValue();
					}
					if (leftn.getClass() == Short.class || rightn.getClass() == Short.class) {
						return leftn.shortValue() + rightn.shortValue();
					}
					if (leftn.getClass() == Byte.class || rightn.getClass() == Byte.class) {
						return leftn.byteValue() + rightn.byteValue();
					}
				}
				throw new TranslationFailedException(
						"Unable to execute operator: " + left + " " + operator + " " + right);
			}
			case "-": {
				if (left == null || right == null) {
					throw new TranslationFailedException(
							"Unable to execute operator: " + left + " " + operator + " " + right);
				}
				if (left.getClass() == Character.class)
					left = (int) left;
				if (right.getClass() == Character.class)
					right = (int) right;
				if (result instanceof Number && right instanceof Number) {
					Number leftn = (Number) result;
					Number rightn = (Number) right;
					if (leftn.getClass() == Double.class || rightn.getClass() == Double.class) {
						return leftn.doubleValue() - rightn.doubleValue();
					}
					if (leftn.getClass() == Float.class || rightn.getClass() == Float.class) {
						return leftn.floatValue() - rightn.floatValue();
					}
					if (leftn.getClass() == Long.class || rightn.getClass() == Long.class) {
						return leftn.longValue() - rightn.longValue();
					}
					if (leftn.getClass() == Integer.class || rightn.getClass() == Integer.class) {
						return leftn.intValue() - rightn.intValue();
					}
					if (leftn.getClass() == Short.class || rightn.getClass() == Short.class) {
						return leftn.shortValue() - rightn.shortValue();
					}
					if (leftn.getClass() == Byte.class || rightn.getClass() == Byte.class) {
						return leftn.byteValue() - rightn.byteValue();
					}
				}
				throw new TranslationFailedException(
						"Unable to execute operator: " + left + " " + operator + " " + right);
			}
			case "<": {
				if (!(left instanceof Number) || !(right instanceof Number)) {
					throw new TranslationFailedException("Operator " + operator + " is not applicable to values: "
							+ left + " " + operator + " " + right);
				}
				Number leftn = (Number) result;
				Number rightn = (Number) right;
				return leftn.doubleValue() < rightn.doubleValue();
			}
			case ">": {
				if (!(left instanceof Number) || !(right instanceof Number)) {
					throw new TranslationFailedException("Operator " + operator + " is not applicable to values: "
							+ left + " " + operator + " " + right);
				}
				Number leftn = (Number) result;
				Number rightn = (Number) right;
				return leftn.doubleValue() > rightn.doubleValue();
			}
			case "<=": {
				if (!(left instanceof Number) || !(right instanceof Number)) {
					throw new TranslationFailedException("Operator " + operator + " is not applicable to values: "
							+ left + " " + operator + " " + right);
				}
				Number leftn = (Number) result;
				Number rightn = (Number) right;
				return leftn.doubleValue() <= rightn.doubleValue();
			}
			case ">=": {
				if (!(left instanceof Number) || !(right instanceof Number)) {
					throw new TranslationFailedException("Operator " + operator + " is not applicable to values: "
							+ left + " " + operator + " " + right);
				}
				Number leftn = (Number) result;
				Number rightn = (Number) right;
				return leftn.doubleValue() >= rightn.doubleValue();
			}
			case "!=": {
				return result != right;
			}
			case "||": {
				try {
					return (Boolean) result || (Boolean) right;
				} catch (ClassCastException e) {
					throw new TranslationFailedException(
							"Failed to cast operands to boolean: left: " + result + " right: " + right);
				}
			}
			case "&&": {
				try {
					return (Boolean) result && (Boolean) right;
				} catch (ClassCastException e) {
					throw new TranslationFailedException(
							"Failed to cast operands to boolean: left: " + result + " right: " + right);
				}
			}
			default: {
				throw new TranslationFailedException("Unrecognized operator: " + operator);
			}
		}
	}

	private static Object evaluateUnaryOperator(Object o, String operator) throws TranslationFailedException {
		if (!(o instanceof Boolean)) {
			throw new TranslationFailedException(
					"Failed to handle unary operator: " + operator + " on non boolean value: " + o);
		}
		switch (operator) {
			case "!": {
				return !(Boolean) o;
			}
			default: {
				throw new TranslationFailedException("Unhandled unary operator: " + operator);
			}
		}
	}

	private static Class<?> lookupClass(String name) throws ClassNotFoundException {
		try {
			return Class.forName(name);
		} catch (ClassNotFoundException e) {
			try {
				return Class.forName("java.lang." + name);
			} catch (ClassNotFoundException e2) {
				try {
					return Class.forName("java.util." + name);
				} catch (ClassNotFoundException e3) {
					throw new ClassNotFoundException("Class not found with name: " + name);
				}
			}
		}
	}

	private static Object evaluateSingleExpression(Statement stm, Stack<VarPair> varstack)
			throws TranslationFailedException {
		try {
			List<Pair<String, Statement>> scopes = stm.getScopes();
			Statement unary = stm.firstScope("unaryoperator");
			Statement child = scopes.get(unary == null ? 0 : 1).value;
			Object result;
			switch (child.getName()) {
				case "parentheses": {
					result = evaluateExpression(child.firstScope("expression"), varstack);
					break;
				}
				case "newexpression": {
					String classname = child.firstValue("class");
					Class<?> clazz = lookupClass(classname);
					Object[] args = child.scopeTo("argument").stream()
							.map((i) -> evaluateExpression(i.firstScope("expression"), varstack)).toArray();
					Constructor<?> constructor = searchConstructor(clazz, args);
					result = constructor.newInstance(args);
					break;
				}
				case "nullliteral": {
					result = null;
					break;
				}
				case "boolliteral": {
					result = Boolean.parseBoolean(child.getValue());
					break;
				}
				case "hexnumberliteral": {
					String val = child.getValue();
					result = Integer.parseUnsignedInt(val, 16);
					break;
				}
				case "binarynumberliteral": {
					String val = child.getValue();
					result = Integer.parseUnsignedInt(val, 2);
					break;
				}
				case "numberliteral": {
					String val = child.getValue();
					result = Integer.parseInt(val, 10);
					break;
				}
				case "stringliteral": {
					result = child.getValue().replace("\\\"", "\"").replace("\\t", "\t").replace("\\n", "\n");
					break;
				}
				case "assignment": {
					String target = child.firstScope("target").getValue();
					Object val = evaluateExpression(child.firstScope("expression"), varstack);
					VarPair found = findPairValue(target, varstack);
					found.value = val;
					result = val;
					break;
				}
				case "exp": {
					String name = child.getValue();
					result = findValue(name, varstack);

					Class<?> objectclass = null;
					if (result == null) {
						if (!"null".equals(name)) {
							try {
								objectclass = lookupClass(name);
							} catch (ClassNotFoundException e) {
								throw new TranslationFailedException(
										stm.getName() + " - " + stm.toDocumentPositionString(), e);
							}
						}
					} else {
						objectclass = result.getClass();
					}

					for (Statement itemscope : child.scopeTo("scope")) {
						if (objectclass == null) {
							throw new TranslationFailedException("Unknown class for statement: " + child);
						}
						Exception accessibleexc = null;
						try {
							String scopename = itemscope.getValue();
							Statement methodcall = itemscope.firstScope("methodcall");
							if (methodcall != null) {
								// method
								Object[] args = methodcall.scopeTo("argument").stream()
										.map((i) -> evaluateExpression(i.firstScope("expression"), varstack)).toArray();
								Method method = searchMethod(scopename, objectclass, args);
								if (!Modifier.isPublic(method.getModifiers())) {
									try {
										method.setAccessible(true);
									} catch (Exception e) {
										accessibleexc = e;
									}
								}
								result = method.invoke(result, args);
								objectclass = result == null ? method.getReturnType() : result.getClass();
							} else {
								// field
								try {
									Field field = searchField(objectclass, scopename);
									if (!Modifier.isPublic(field.getModifiers())) {
										field.setAccessible(true);
										try {
											field.setAccessible(true);
										} catch (Exception e) {
											accessibleexc = e;
										}
									}
									result = field.get(result);
									objectclass = result == null ? field.getType() : result.getClass();
								} catch (NoSuchFieldException e) {
									throw new NoSuchFieldException(
											"No field " + e.getMessage() + " in class: " + objectclass);
								}
							}
						} catch (Exception e) {
							if (accessibleexc != null) {
								e.addSuppressed(accessibleexc);
							}
							throw e;
						}
					}

					break;
				}
				default: {
					throw new TranslationFailedException("Unhandled child of expression: " + child.getName());
				}
			}
			return unary == null ? result : evaluateUnaryOperator(result, unary.getValue());
		} catch (Exception e) {
			throw new TranslationFailedException(e);
		}
	}

	private static String[][] PRECEDENCE_TABLE = { { "*", "/", "%" }, { "+", "-" }, { "<<", ">>", ">>>" },
			{ "<", ">", "<=", ">=", "instanceof" }, { "==", "!=" }, { "&" }, { "^" }, { "|" }, { "&&" }, { "||" } };

	private static int getPrecedence(String operator) {
		for (int i = 0; i < PRECEDENCE_TABLE.length; i++) {
			for (int j = 0; j < PRECEDENCE_TABLE[i].length; j++) {
				if (PRECEDENCE_TABLE[i][j].equals(operator)) {
					return i;
				}
			}
		}
		throw new TranslationFailedException("Precedence not found for operator: " + operator);
	}

	private static Object evaluateExpression(Statement stm, Stack<VarPair> varstack) {
		Object left = evaluateSingleExpression(stm, varstack);
		Statement operator = stm.firstScope("operator");
		while (operator != null) {
			int percedence = getPrecedence(operator.getValue());
			Statement rightstm = operator.firstScope("expression");
			Statement rightoperatorstm = rightstm.firstScope("operator");
			if (rightoperatorstm == null || percedence <= getPrecedence(rightoperatorstm.getValue())) {
				// do it now
				Object right = evaluateSingleExpression(rightstm, varstack);
				left = executeOperator(operator.getValue(), left, right);
				operator = rightoperatorstm;
			} else {
				// execute after we got the right
				Object right = evaluateExpression(rightstm, varstack);
				left = executeOperator(operator.getValue(), left, right);
				break;
			}
		}
		return left;
	}

	private class FunctionAction {
		Stack<VarPair> funclocals;
		String name;
		String[] args;
		Statement body;

		public FunctionAction(Stack<VarPair> funclocals, String name, String[] args, Statement body) {
			this.funclocals = new Stack<>();
			this.funclocals.addAll(funclocals);
			this.name = name;
			this.args = args;
			this.body = body;
		}

		public void call(Object[] args, OutputStream output) throws TranslationFailedException, IOException {
			Stack<VarPair> valmapstack = new Stack<>();
			valmapstack.addAll(funclocals);
			for (int i = 0; i < args.length; i++) {
				valmapstack.push(new VarPair(this.args[i], args[i]));
			}
			translate(body, output, valmapstack);
		}
	}

	private void translate(Statement stm, OutputStream output, Stack<VarPair> varstack)
			throws TranslationFailedException, IOException {
		int startsize = varstack.size();

		try {
			for (Pair<String, Statement> scope : stm.getScopes()) {
				switch (scope.key) {
					case "whitespacetext": {
						wsbuffer.append(scope.value.getValue());
						break;
					}
					case "text": {
						int nlineindex = wsbuffer.lastIndexOf("\n");
						if (nlineindex > 0) {
							wsbuffer.delete(0, nlineindex);
						}
						output.write(wsbuffer.toString().getBytes());
						// output.print(wsbuffer.toString());
						wsbuffer.setLength(0);
						output.write(scope.value.getValue().getBytes());
						// output.print(scope.value.getValue());
						break;
					}
					case "variable": {
						String varname = scope.value.firstValue("name");
						Statement initer = scope.value.firstScope("initializer");
						Object value = null;
						if (initer != null) {
							value = evaluateExpression(initer.firstScope("expression"), varstack);
						}
						varstack.push(new VarPair(varname, value));
						break;
					}
					case "valueaction": {
						Statement optstm = scope.value.firstScope("optional");
						Statement expressionstm = scope.value.firstScope("expression");

						try {
							Object itemval = evaluateExpression(expressionstm, varstack);
							if (itemval == null) {
								throw new TranslationFailedException(
										"Expression is not printable: " + itemval + " " + varstack);
							}
							if (expressionstm.firstScope("assignment") == null
									&& !"void".equals(scope.value.firstValue("optional"))) {
								// only write if it wasnt assignment
								int nlineindex = wsbuffer.lastIndexOf("\n");
								if (nlineindex > 0) {
									wsbuffer.delete(0, nlineindex);
								}
								output.write(wsbuffer.toString().getBytes());
								// output.print(wsbuffer.toString());
								wsbuffer.setLength(0);
								if (itemval instanceof SourceWritable) {
									((SourceWritable) itemval).write(output);
								} else {
									output.write(itemval.toString().getBytes());
									// output.print(itemval);
								}
							}
						} catch (Exception e) {
							if (optstm == null) {
								// not optional value, report exception
								throw e;
							}
						}
						break;
					}
					case "replaceaction": {
						String keyval = scope.value.firstValue("key");
						if (handler == null) {
							throw new TranslationFailedException(
									"TranslationHandler is null, and found replace " + keyval);
						}
						handler.replaceKey(keyval, output);
						break;
					}
					case "includeaction": {
						String keyval = scope.value.firstValue("key");
						List<Statement> paramscopes = scope.value.scopeTo("parameter");
						for (Statement param : paramscopes) {
							varstack.push(new VarPair(param.firstValue("name"),
									evaluateExpression(param.firstScope("value").firstScope("expression"), varstack)));
						}
						if (includeResolver == null) {
							throw new FileNotFoundException(
									"Failed to resolve include path: " + keyval + ", include resolver is null");
						}
						InputStream included = includeResolver.get(keyval);
						if (included == null) {
							throw new FileNotFoundException("Failed to resolve include: " + keyval);
						}
						int nlineindex = wsbuffer.lastIndexOf("\n");
						if (nlineindex > 0) {
							wsbuffer.delete(0, nlineindex);
						}
						output.write(wsbuffer.toString().getBytes());
						// output.print(wsbuffer.toString());
						wsbuffer.setLength(0);
						try (InputStream is = included) {
							translate(is, output,
									varstack.stream().collect(Collectors.toMap(v -> v.name, v -> v.value)),
									handler == null ? null : handler.includeFile(keyval, output), includeResolver);
						} finally {
							for (int i = 0; i < paramscopes.size(); i++) {
								varstack.pop();
							}
						}

						break;
					}
					case "foreachaction": {
						List<Object> values = new ArrayList<>();
						for (Statement collstm : scope.value.scopeTo("collection")) {
							Object val = evaluateExpression(collstm.firstScope("expression"), varstack);
							if (val.getClass().isArray()) {
								for (Object o : (Object[]) val) {
									values.add(o);
								}
							} else if (val instanceof Iterable<?>) {
								for (Object o : (Iterable<?>) val) {
									values.add(o);
								}
							} else {
								throw new TranslationFailedException(
										"Foreach object must be Iterable<?> or array when using @foreach: \""
												+ val.getClass() + "\"");
							}
						}
						String itemname = scope.value.firstValue("itemname");
						for (Object o : values) {
							varstack.push(new VarPair(itemname, o));
							translate(scope.value.firstScope("data"), output, varstack);
							varstack.pop();
						}
						break;
					}
					case "whileaction": {
						Statement conditionstm = scope.value.firstScope("condition");
						while ((Boolean) evaluateExpression(conditionstm, varstack)) {
							translate(scope.value.firstScope("data"), output, varstack);
						}
						break;
					}
					case "dowhileaction": {
						Statement conditionstm = scope.value.firstScope("condition");
						do {
							translate(scope.value.firstScope("data"), output, varstack);
						} while ((Boolean) evaluateExpression(conditionstm, varstack));
						break;
					}
					case "ifelseaction": {
						Object condition = evaluateExpression(scope.value.firstScope("condition"), varstack);
						if (condition == null
								|| (condition.getClass() != Boolean.class && condition.getClass() != boolean.class)) {
							throw new TranslationFailedException(
									"If condition did not evaluate to boolean: " + condition);
						}

						if ((boolean) condition) {
							translate(scope.value.firstScope("truebranch"), output, varstack);
						} else {
							Statement elsebranch = scope.value.firstScope("elsebranch");
							if (elsebranch != null) {
								translate(elsebranch, output, varstack);
							}
						}
						break;
					}
					case "declarefunctionaction": {
						String name = scope.value.firstValue("name");
						String[] args = scope.value.scopeTo("argument").stream().map(a -> a.getValue())
								.toArray(String[]::new);
						Statement body = scope.value.firstScope("data");
						FunctionAction funcaction = new FunctionAction(varstack, name, args, body);
						funcaction.funclocals.push(new VarPair(FUNCTION_PREFIX + funcaction.name, funcaction));
						varstack.push(new VarPair(FUNCTION_PREFIX + funcaction.name, funcaction));
						break;
					}
					case "callfunctionaction": {
						String name = scope.value.firstValue("name");
						Object[] args = scope.value.scopeTo("argument").stream()
								.map(a -> evaluateExpression(a, varstack)).toArray();
						FunctionAction func = (FunctionAction) findValue(FUNCTION_PREFIX + name, varstack);
						if (func == null) {
							throw new TranslationFailedException("Function not found with name: " + name);
						}
						if (func.args.length != args.length) {
							throw new TranslationFailedException("Function " + name + " requires " + func.args.length
									+ " arguments, " + args.length + " provided");
						}
						func.call(args, output);
						break;
					}
					default: {
						break;
					}
				}
			}
		} finally {
			while (varstack.size() > startsize) {
				varstack.pop();
			}
		}
	}

	public static void translate(InputStream input, OutputStream output, Map<String, Object> valuemap,
			TranslationHandler handler, IncludeResolver includeresolver) throws IOException {
		if (valuemap == null) {
			valuemap = Collections.emptyMap();
		}
		Statement parsed = null;
		try {
			Stack<VarPair> stack = new Stack<>();
			for (Entry<String, Object> entry : valuemap.entrySet()) {
				stack.push(new VarPair(entry.getKey(), entry.getValue()));
			}
			parsed = TEMPLATE_LANG.parseInputStream(input).getStatement();
			new SourceTemplateTranslator(handler, includeresolver).translate(parsed, output, stack);
		} catch (TranslationFailedException | ParseFailedException e) {
			if (parsed != null) {
				System.out.println("Error trace:");
				parsed.prettyprint(System.out);
			}
			throw new IOException(e);
		}
	}

}
