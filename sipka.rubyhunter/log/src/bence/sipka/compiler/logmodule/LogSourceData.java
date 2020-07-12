package bence.sipka.compiler.logmodule;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import java.util.Map;

import bence.sipka.compiler.types.TypeDeclaration;
import saker.build.task.utils.annot.SakerInput;
import saker.build.thirdparty.saker.util.io.SerialUtils;

public class LogSourceData implements Externalizable {
	private static final long serialVersionUID = 1L;

	@SakerInput(value = "Debug")
	public boolean Debug = false;

	@SakerInput(value = "DebugNew")
	public boolean debugNew = false;

	@SakerInput(value = "SupportDebugNew")
	public boolean supportDebugNew = false;

	@SakerInput(value = "Enabled")
	public Boolean enabled = null;

	@SakerInput(value = "LogDebugNew")
	public boolean logDebugNew = true;

	@SakerInput("PlatformName")
	public String PlatformName;

	public Map<String, TypeDeclaration> typeDeclarations;

	/**
	 * For {@link Externalizable}.
	 */
	public LogSourceData() {
	}

	@Override
	public void writeExternal(ObjectOutput out) throws IOException {
		out.writeBoolean(Debug);
		out.writeBoolean(debugNew);
		out.writeBoolean(supportDebugNew);
		out.writeObject(enabled);
		out.writeBoolean(logDebugNew);
		out.writeObject(PlatformName);
		SerialUtils.writeExternalMap(out, typeDeclarations);
	}

	@Override
	public void readExternal(ObjectInput in) throws IOException, ClassNotFoundException {
		Debug = in.readBoolean();
		debugNew = in.readBoolean();
		supportDebugNew = in.readBoolean();
		enabled = SerialUtils.readExternalObject(in);
		logDebugNew = in.readBoolean();
		PlatformName = SerialUtils.readExternalObject(in);
		typeDeclarations = SerialUtils.readExternalImmutableNavigableMap(in);
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + (Debug ? 1231 : 1237);
		result = prime * result + ((PlatformName == null) ? 0 : PlatformName.hashCode());
		result = prime * result + (debugNew ? 1231 : 1237);
		result = prime * result + ((enabled == null) ? 0 : enabled.hashCode());
		result = prime * result + (logDebugNew ? 1231 : 1237);
		result = prime * result + (supportDebugNew ? 1231 : 1237);
		result = prime * result + ((typeDeclarations == null) ? 0 : typeDeclarations.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		LogSourceData other = (LogSourceData) obj;
		if (Debug != other.Debug)
			return false;
		if (PlatformName == null) {
			if (other.PlatformName != null)
				return false;
		} else if (!PlatformName.equals(other.PlatformName))
			return false;
		if (debugNew != other.debugNew)
			return false;
		if (enabled == null) {
			if (other.enabled != null)
				return false;
		} else if (!enabled.equals(other.enabled))
			return false;
		if (logDebugNew != other.logDebugNew)
			return false;
		if (supportDebugNew != other.supportDebugNew)
			return false;
		if (typeDeclarations == null) {
			if (other.typeDeclarations != null)
				return false;
		} else if (!typeDeclarations.equals(other.typeDeclarations))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "LogSourceData[Debug=" + Debug + ", debugNew=" + debugNew + ", supportDebugNew=" + supportDebugNew
				+ ", " + (enabled != null ? "enabled=" + enabled + ", " : "") + "logDebugNew=" + logDebugNew + ", "
				+ (PlatformName != null ? "PlatformName=" + PlatformName + ", " : "")
				+ (typeDeclarations != null ? "typeDeclarations=" + typeDeclarations : "") + "]";
	}

}